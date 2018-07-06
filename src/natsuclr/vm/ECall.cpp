//
// Natsu CLR VM
//
#include <vm/ECall.hpp>
#include <utils.hpp>
#include <cstring>
#include <vm/EvaluationStack.hpp>
#include <classlibnative/System/Console.hpp>

using namespace clr;
using namespace clr::vm;

template<typename Func>
struct func_info;

template<size_t I, typename ...TArgs>
struct arg_info
{
	using Type = std::tuple_element_t<I, std::tuple<TArgs...>>;
};

template<typename TRet, typename... TArgs>
struct func_info<TRet(*)(TArgs...)>
{
	static_assert(std::is_void_v<TRet>, "Now just support void return.");

	static const auto ArgsCount = sizeof...(TArgs);

	static void Call(uintptr_t entryPoint, CalleeInfo& callee)
	{
		CallImp(entryPoint, callee, std::make_index_sequence<ArgsCount>());
	}
private:
	template<size_t N>
	static auto ArgAt(CalleeInfo& callee)
	{
		using arg_t = arg_info<N, TArgs...>;
		return *reinterpret_cast<typename arg_t::Type*>(callee.GetArg(N).Data);
	}

	template<size_t ...ArgIdx>
	static auto CallImp(uintptr_t entryPoint, CalleeInfo& callee, std::index_sequence<ArgIdx...>)
	{
		auto func = reinterpret_cast<TRet(*)(TArgs...)>(entryPoint);
		func(ArgAt<ArgIdx>(callee)...);
	}
};

#define ECFuncStart(funcvar, ns, cls) static const ECall funcvar[] = {
#define ECFuncElement(name, func) { name, uintptr_t(func), &func_info<decltype(func)>::Call },
#define ECFuncEnd() { nullptr, 0, 0 }};

#include <vm/ECallList.hpp>

#define ECFuncStart(funcvar, ns, cls) { ns, cls, funcvar },
#define ECFuncElement(name, func)
#define ECFuncEnd()

static const ECallClass g_ECalls[] =
{
#include <vm/ECallList.hpp>
	{ nullptr, nullptr, nullptr }
};

const ECall& clr::vm::FindECall(const MethodDesc& method)
{
	bool found = false;

	const ECallClass* ecallClass = g_ECalls;

	{
		auto ns = method.Class->TypeNamespace;
		auto cls = method.Class->TypeName;
		while (ecallClass->Namespace)
		{
			if (strcmp(ns, ecallClass->Namespace) == 0)
			{
				if (strcmp(cls, ecallClass->ClassName) == 0)
				{
					found = true;
					break;
				}
			}
		}
	}

	if (found)
	{
		auto name = method.Name;
		const ECall* ecall = ecallClass->ECalls;
		while (ecall->MethodName)
		{
			if (strcmp(name, ecall->MethodName) == 0)
				return *ecall;
		}
	}

	THROW_ALWAYS(ECallNotFoundException);
}
