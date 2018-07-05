//
// Natsu CLR Loader
//
#include <utils.hpp>
#include <loader/AssemblyLoader.hpp>
#include <vm/ECall.hpp>
#include <md/Signature.hpp>
#include <cassert>
#include <map>

using namespace clr::loader;
using namespace clr::metadata;
using namespace clr::vm;

AssemblyLoader::AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile)
	:mdImporter_(assemblyFile), assemblyFile_(assemblyFile)
{

}

void AssemblyLoader::Load()
{
	auto typeDefs = mdImporter_.GetTables().GetRowsCount(mdt_TypeDef);
	eeClasses_.resize(typeDefs);
	auto methodDefs = mdImporter_.GetTables().GetRowsCount(mdt_MethodDef);
	methodDescs_.resize(methodDefs);
	auto fields = mdImporter_.GetTables().GetRowsCount(mdt_Field);
	fieldDescs_.resize(fields);

	for (size_t i = 0; i < typeDefs; i++)
		LoadTypeDef(i);

	for (size_t i = 0; i < methodDefs; i++)
		LoadMethodDef(i);

	for (size_t i = 0; i < fields; i++)
		LoadField(i);

	for (size_t i = 0; i < typeDefs; i++)
		LoadTypeInstanceField(i);
	for (size_t i = 0; i < typeDefs; i++)
		LoadTypeStaticField(i);
}

void AssemblyLoader::LoadTypeDef(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();

	auto&& eeClass = eeClasses_[index];

	eeClass.MDImporter = &mdImporter_;

	auto typeDef = tables.GetTypeDef({ index + 1 });
	eeClass.TypeName = strings.GetString(typeDef.TypeName);
	eeClass.TypeNamespace = strings.GetString(typeDef.TypeNamespace);

	auto hasNextType = index + 1 < tables.GetRowsCount(mdt_TypeDef);

	if (typeDef.MethodList)
	{
		eeClass.FirstMethod = methodDescs_.data() + typeDef.MethodList() - 1;
		if (hasNextType)
			eeClass.LastMethod = methodDescs_.data() + tables.GetTypeDef({ index + 2 }).MethodList() - 1;
		else
			eeClass.LastMethod = methodDescs_.data() + methodDescs_.size();

		for (auto method = eeClass.FirstMethod; method != eeClass.LastMethod; method++)
			method->Class = &eeClass;
	}

	if (typeDef.FieldList)
	{
		eeClass.FirstField = fieldDescs_.data() + typeDef.FieldList() - 1;
		if (hasNextType)
			eeClass.LastField = fieldDescs_.data() + tables.GetTypeDef({ index + 2 }).FieldList() - 1;
		else
			eeClass.LastField = fieldDescs_.data() + fieldDescs_.size();

		for (auto field = eeClass.FirstField; field != eeClass.LastField; field++)
			field->Class = &eeClass;
	}
}

void AssemblyLoader::LoadMethodDef(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();

	auto&& method = methodDescs_[index];

	auto methodDef = tables.GetMethodDef({ index + 1 });
	method.Name = strings.GetString(methodDef.Name);

	if ((methodDef.ImplFlags & MethodImplAttributes::InternalCall) == MethodImplAttributes::InternalCall)
	{
		method.IsECall = true;

		auto& ecall = FindECall(method);
		method.ECall.EntryPoint = ecall.EntryPoint;
		method.ECall.ParamsCount = ecall.ParamsCount;
	}
	else
	{
		method.IsECall = false;
		auto headerOffset = assemblyFile_->GetDataByRVA(methodDef.RVA);
		BinaryReader br((uintptr_t)headerOffset);
		{
			enum
			{
				CorILMethod_TinyFormat = 0x2,
				CorILMethod_FatFormat = 0x3,
				CorILMethod_FormatMask = 0x3
			};

			auto flag = br.Read<uint8_t>();
			auto format = flag & CorILMethod_FormatMask;
			size_t bodyLength;
			if (format == CorILMethod_TinyFormat)
			{
				bodyLength = flag >> 2;

				method.MaxStack = 8;
				method.BodyBegin = headerOffset + 1;
			}
			else if (format == CorILMethod_FatFormat)
			{
				uint16_t fatFloags = (br.Read<uint8_t>() << 8) | flag;
				auto headerSize = (fatFloags >> 12) * 4;
				method.MaxStack = br.Read<uint16_t>();
				bodyLength = br.Read<uint32_t>();

				method.BodyBegin = headerOffset + headerSize;
			}
			else
			{
				THROW_ALWAYS(BadImageException, "Invalid method header.");
			}

			method.BodyEnd = method.BodyBegin + bodyLength;
		}
	}
}

class FieldSigVisitor : public SignatureVisitor
{
public:
	AssemblyLoader * AssemblyLoader;

	size_t Size = 0;
	bool GetSizeFromType;

	virtual void VisitBeginType(CorElementType elementType) override
	{
		uint32_t size;
		switch (elementType)
		{
		case ELEMENT_TYPE_BOOLEAN:
		case ELEMENT_TYPE_CHAR:
		case ELEMENT_TYPE_I1:
		case ELEMENT_TYPE_U1:
			size = 1;
			break;
		case ELEMENT_TYPE_U2:
		case ELEMENT_TYPE_I2:
			size = 2;
			break;
		case ELEMENT_TYPE_I4:
		case ELEMENT_TYPE_U4:
		case ELEMENT_TYPE_R4:
			size = 4;
			break;
		case ELEMENT_TYPE_I8:
		case ELEMENT_TYPE_U8:
		case ELEMENT_TYPE_R8:
			size = 8;
			break;
		case ELEMENT_TYPE_I:
		case ELEMENT_TYPE_U:
		case ELEMENT_TYPE_STRING:
		case ELEMENT_TYPE_OBJECT:
		case ELEMENT_TYPE_CLASS:
			size = sizeof(uintptr_t);
			break;
		case ELEMENT_TYPE_VALUETYPE:
			GetSizeFromType = true;
			size = 0;
			break;
		default:
			THROW_ALWAYS(NotSupportedException);
			break;
		}

		Size = size;
	}

	virtual void VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx) override
	{
		assert(cridx.GetType() == mdt_TypeDef);
		auto classId = cridx.As<mdt_TypeDef>();
		auto& eeClass = AssemblyLoader->GetClasses()[classId];
		if (eeClass.LoadLevel < clsLoad_InstanceFields)
			AssemblyLoader->LoadTypeInstanceField(classId() - 1);

		Size = eeClass.InstanceSize;
	}
};

void AssemblyLoader::LoadField(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();

	auto& fieldDesc = fieldDescs_[index];
	auto field = tables.GetField({ index + 1 });
	fieldDesc.Ridx = { index + 1 };
	fieldDesc.Name = strings.GetString(field.Name);
	fieldDesc.Flags = field.Flags;
}

static void CalcFieldSize(AssemblyLoader* assemblyLoader, FieldDesc& fieldDesc, const MetadataStream& tables, MDImporter& mdImporter)
{
	auto field = tables.GetField(fieldDesc.Ridx);

	Signature sig(mdImporter.GetBlobs().GetBlob(field.Signature));
	auto sigParser = sig.CreateParser();
	FieldSigVisitor visitor;
	visitor.AssemblyLoader = assemblyLoader;
	visitor.Parse(sigParser);
	fieldDesc.Size = visitor.Size;
}

static uint32_t CalcFieldOffset(const std::multimap<uint32_t, FieldDesc*>& fieldSize)
{
	uint32_t offset = 0;
	uint32_t lastSize = 0;
	for (auto fieldPair = fieldSize.begin(); fieldPair != fieldSize.end(); ++fieldPair)
	{
		if (lastSize != fieldPair->first)
		{
			lastSize = fieldPair->first;
			offset = align(offset, lastSize);
		}

		fieldPair->second->Offset = offset;
		offset += lastSize;
	}

	return offset;
}

void AssemblyLoader::LoadTypeInstanceField(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();
	auto& eeClass = eeClasses_[index];

	for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
		if ((fieldDesc->Flags & FieldAttributes::Static) != FieldAttributes::Static)
			CalcFieldSize(this, *fieldDesc, tables, mdImporter_);

	for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
		if ((fieldDesc->Flags & FieldAttributes::Static) == FieldAttributes::Static)
			CalcFieldSize(this, *fieldDesc, tables, mdImporter_);

	std::multimap<uint32_t, FieldDesc*> fieldSize;

	// Instance field
	{
		for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
			if ((fieldDesc->Flags & FieldAttributes::Static) != FieldAttributes::Static)
				fieldSize.emplace(fieldDesc->Size, fieldDesc);

		eeClass.InstanceSize = CalcFieldOffset(fieldSize);
	}

	eeClass.LoadLevel = clsLoad_InstanceFields;
}

void AssemblyLoader::LoadTypeStaticField(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();
	auto& eeClass = eeClasses_[index];

	std::multimap<uint32_t, FieldDesc*> fieldSize;

	// Static & not literal field
	{
		for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
		{
			auto flag = fieldDesc->Flags;
			if ((flag & FieldAttributes::Static) == FieldAttributes::Static &&
				(flag & FieldAttributes::Literal) != FieldAttributes::Literal)
				fieldSize.emplace(fieldDesc->Size, fieldDesc);
		}

		eeClass.StaticSize = CalcFieldOffset(fieldSize);
		if (eeClass.StaticSize)
			eeClass.StaticFields = std::make_unique<uint8_t[]>(eeClass.StaticSize);
	}

	eeClass.LoadLevel = clsLoad_StaticFields;
}

const MethodDesc& AssemblyLoader::GetMethod(Ridx<mdt_MethodDef> method) const
{
	assert(method);
	return methodDescs_[method() - 1];
}

const EEClass& AssemblyLoader::GetClass(Ridx<mdt_TypeDef> type) const
{
	assert(type);
	return eeClasses_[type() - 1];
}
