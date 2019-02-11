//
// Natsu CLR VM
//
#include <classlibnative/System/Array.hpp>
#include <classlibnative/System/Console.hpp>
#include <classlibnative/System/Object.hpp>
#include <cassert>
#include <cstring>
#include <utils.hpp>
#include <vm/ECall.hpp>
#include <vm/EvaluationStack.hpp>

using namespace clr;
using namespace clr::vm;

template <typename Func>
struct func_info;

template <size_t I, typename... TArgs>
struct arg_info
{
    using Type = std::tuple_element_t<I, std::tuple<TArgs...>>;
};

template <typename TRet, typename... TArgs>
struct func_info<TRet (*)(TArgs...)>
{
    static const auto ArgsCount = sizeof...(TArgs);

    static void Call(uintptr_t entryPoint, CalleeInfo& callee)
    {
        if constexpr (std::is_void_v<TRet>)
        {
            return CallImp(entryPoint, callee, std::make_index_sequence<ArgsCount>());
        }
        else
        {
            TypeDesc retType;
            auto dest = callee.GetRet(retType);
            assert(retType.GetStackSize() == align(sizeof(TRet), 8));
            auto ret = CallImp(entryPoint, callee, std::make_index_sequence<ArgsCount>());
            memcpy(dest, &ret, sizeof(TRet));
        }
    }

private:
    template <size_t N>
    static auto ArgAt(CalleeInfo& callee)
    {
        using arg_t = arg_info<N, TArgs...>;
        TypeDesc srcType;
        return *reinterpret_cast<typename arg_t::Type*>(callee.GetArg(N, srcType));
    }

    template <size_t... ArgIdx>
    static auto CallImp(uintptr_t entryPoint, CalleeInfo& callee, std::index_sequence<ArgIdx...>)
    {
        auto func = reinterpret_cast<TRet (*)(TArgs...)>(entryPoint);
        return func(ArgAt<ArgIdx>(callee)...);
    }
};

#define ECFuncStart(funcvar, ns, cls) static const ECall funcvar[] = {
#define ECFuncElement(name, func) { name, uintptr_t(func), &func_info<decltype(func)>::Call },
#define ECFuncEnd() \
    {               \
        {}, 0, 0    \
    }               \
    }               \
    ;

#include <vm/ECallList.hpp>

#define ECFuncStart(funcvar, ns, cls) { ns, cls, funcvar },
#define ECFuncElement(name, func)
#define ECFuncEnd()

static const ECallClass g_ECalls[] = {
#include <vm/ECallList.hpp>
    { {}, {}, nullptr }
};

const ECall& clr::vm::FindECall(const MethodDesc& method)
{
    bool found = false;

    const ECallClass* ecallClass = g_ECalls;

    {
        auto ns = method.Class->TypeNamespace;
        auto cls = method.Class->TypeName;
        while (!ecallClass->Namespace.empty())
        {
            if (ecallClass->Namespace == ns)
            {
                if (ecallClass->ClassName == cls)
                {
                    found = true;
                    break;
                }
            }

            ecallClass++;
        }
    }

    if (found)
    {
        auto name = method.Name;
        const ECall* ecall = ecallClass->ECalls;
        while (!ecall->MethodName.empty())
        {
            if (ecall->MethodName == name)
                return *ecall;
            ecall++;
        }
    }

    THROW_ALWAYS(ECallNotFoundException);
}
