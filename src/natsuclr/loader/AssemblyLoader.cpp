//
// Natsu CLR Loader
//
#include <loader/AssemblyLoader.hpp>
#include <utils.hpp>

using namespace clr::loader;
using namespace clr::metadata;
using namespace clr::vm;

AssemblyLoader::AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile)
	:mdImporter_(std::move(assemblyFile))
{

}

static void LoadType(const MetadataStream& tables, const StringsStream& strings, size_t index, std::vector<EEClass>& eeClasses)
{
	auto&& eeClass = eeClasses[index];
	if (eeClass.TypeName)return;

	auto typeDef = tables.GetTypeDef({ index + 1 });
	eeClass.TypeName = strings.GetString(typeDef.TypeName);
	eeClass.TypeNamespace = strings.GetString(typeDef.TypeNamespace);
}

void AssemblyLoader::Load()
{
	auto& tables = mdImporter_.GetTables();
	auto& strings = mdImporter_.GetStrings();
	auto typeDefs = tables.GetRowsCount(mdt_TypeDef);
	eeClasses_.resize(typeDefs);
	for (size_t i = 0; i < typeDefs; i++)
		LoadType(tables, strings, i, eeClasses_);
}
