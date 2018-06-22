//
// Natsu CLR Loader
//
#include <loader/AssemblyLoader.hpp>
#include <utils.hpp>

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
	eeMethod.Body = assemblyFile_->GetDataByRVA(methodDef.RVA);
}
