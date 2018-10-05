//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"
#include "EEClass.hpp"

namespace clr
{
namespace vm
{
    class ECallNotFoundException : std::exception
    {
        using exception::exception;
    };

    class EvaluationStack;

    struct ECall
    {
        std::string_view MethodName;

        uintptr_t EntryPoint;
        ECallInvoker Call;
    };

    struct ECallClass
    {
        std::string_view Namespace;
        std::string_view ClassName;
        const ECall* ECalls;
    };

    const ECall& FindECall(const MethodDesc& method);
}
}
