//
// Natsu CLR Binder
//
#include <binder/Binder.hpp>
#include <cstring>

using namespace clr;
using namespace clr::loader;
using namespace clr::vm;

Binder::Binder(std::shared_ptr<AssemblyLoader> assemblyLoader)
    : assemblyLoader_(assemblyLoader)
{
}

vm::MethodDesc* Binder::BindMethod(std::string_view typeNameSpace, std::string_view typeName, std::string_view methodName)
{
    if (auto type = BindType(typeNameSpace, typeName))
    {
        for (auto mit = type->FirstMethod; mit != type->LastMethod; ++mit)
        {
            if (mit->Name == methodName)
                return mit;
        }
    }

    return nullptr;
}

vm::EEClass* Binder::BindType(std::string_view typeNameSpace, std::string_view typeName)
{
    for (auto& type : assemblyLoader_->GetClasses())
    {
        if (type.TypeNamespace == typeNameSpace && type.TypeName == typeName)
            return &type;
    }

    return nullptr;
}
