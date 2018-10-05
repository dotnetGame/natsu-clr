//
// Natsu CLR Binder
//
#pragma once
#include "../loader/AssemblyLoader.hpp"
#include <string_view>
#include <optional>

namespace clr
{
class Binder
{
public:
    Binder(std::shared_ptr<loader::AssemblyLoader> assemblyLoader);

    vm::MethodDesc* BindMethod(std::string_view typeNameSpace, std::string_view typeName, std::string_view methodName);
    vm::EEClass* BindType(std::string_view typeNameSpace, std::string_view typeName);

private:
    std::shared_ptr<loader::AssemblyLoader> assemblyLoader_;
};
}
