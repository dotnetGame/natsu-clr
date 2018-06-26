//
// Natsu CLR Loader
//
#include <loader/AssemblyLoader.hpp>
#include <utils.hpp>
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
	eeMethods_.resize(methodDefs);

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
		eeClass.FirstMethod = eeMethods_.data() + typeDef.MethodList() - 1;
		if (hasNextType)
			eeClass.LastMethod = eeMethods_.data() + tables.GetTypeDef({ index + 2 }).MethodList() - 1;
		else
			eeClass.LastMethod = eeMethods_.data() + eeMethods_.size();
	}
}

void AssemblyLoader::LoadMethodDef(size_t index)
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();

	auto&& eeMethod = eeMethods_[index];

	eeMethod.MDImporter = &mdImporter_;

	auto methodDef = tables.GetMethodDef({ index + 1 });
	eeMethod.Name = strings.GetString(methodDef.Name);

	BinaryReader br(uintptr_t(assemblyFile_->GetDataByRVA(methodDef.RVA)));
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
		}
		else if(format == CorILMethod_FatFormat)
		{
			assert(!"Not implemented.");
		}
		else
		{
			THROW_ALWAYS(BadImageException, "Invalid method header.");
		}

		eeMethod.BodyBegin = reinterpret_cast<const uint8_t*>(br.GetOffset());
		eeMethod.BodyEnd = eeMethod.BodyBegin + bodyLength;
	}
}
