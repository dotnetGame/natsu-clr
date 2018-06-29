//
// Natsu CLR Loader
//
#include <utils.hpp>
#include <loader/AssemblyLoader.hpp>
#include <cassert>

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

	for (size_t i = 0; i < typeDefs; i++)
		LoadTypeDef(i);

	for (size_t i = 0; i < methodDefs; i++)
		LoadMethodDef(i);
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

	if (typeDef.MethodList)
	{
		auto hasNextType = index + 1 < tables.GetRowsCount(mdt_TypeDef);
		eeClass.FirstMethod = methodDescs_.data() + typeDef.MethodList() - 1;
		if (hasNextType)
			eeClass.LastMethod = methodDescs_.data() + tables.GetTypeDef({ index + 2 }).MethodList() - 1;
		else
			eeClass.LastMethod = methodDescs_.data() + methodDescs_.size();
	}
}

void AssemblyLoader::LoadMethodDef(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();

	auto&& method = methodDescs_[index];

	method.MDImporter = &mdImporter_;

	auto methodDef = tables.GetMethodDef({ index + 1 });
	method.Name = strings.GetString(methodDef.Name);

	if ((methodDef.ImplFlags & MethodImplAttributes::InternalCall) == MethodImplAttributes::InternalCall)
	{
		method.IsECall = true;
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
