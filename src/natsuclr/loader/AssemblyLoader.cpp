//
// Natsu CLR Loader
//
#include <loader/AssemblyLoader.hpp>
#include <utils.hpp>

using namespace clr::loader;

AssemblyLoader::AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile)
	:mdImporter_(std::move(assemblyFile))
{

}

void AssemblyLoader::Load()
{

}
