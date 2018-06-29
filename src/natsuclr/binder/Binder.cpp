//
// Natsu CLR Binder
//
#include <binder/Binder.hpp>
#include <cstring>

using namespace clr;
using namespace clr::loader;
using namespace clr::vm;

Binder::Binder(std::shared_ptr<AssemblyLoader> assemblyLoader)
	:assemblyLoader_(assemblyLoader)
{

}

vm::MethodDesc* Binder::BindMethod(const char* typeName, const char* methodName)
{
	for (auto& type : assemblyLoader_->GetClasses())
	{
		if (strcmp(typeName, type.TypeName) == 0)
		{
			for (auto mit = type.FirstMethod; mit != type.LastMethod; ++mit)
			{
				if (strcmp(methodName, mit->Name) == 0)
					return mit;
			}
		}
	}

	return nullptr;
}
