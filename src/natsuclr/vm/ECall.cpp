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

template<size_t I, size_t N, typename ...TArgs>
struct arg_info;

template<size_t I, size_t N, class TArg>
struct arg_info<I, N, TArg>
{
	using Type = TArg;
	static constexpr auto Offset = I >= N ? align(sizeof(TArg), sizeof(uintptr_t)) / sizeof(uintptr_t) : 0;
};

template<size_t I, size_t N, typename TArg, typename... TArgs>
struct arg_info<I, N, TArg, TArgs...>
{
	using Type = TArg;
	static constexpr auto Offset = arg_info<I, N, TArg>::Offset + arg_info<I + 1, N, TArgs...>::Offset;
};

template<typename TRet, typename... TArgs>
struct func_info<TRet(*)(TArgs...)>
{
	static_assert(std::is_void_v<TRet>, "Now just support void return.");

	static const auto ArgsCount = sizeof...(TArgs);
	static const auto ArgsSize = arg_info<0, 0, TArgs...>::Offset;

	static void Call(uintptr_t entryPoint, EvaluationStack& stack)
	{
		CallImp(entryPoint, stack, std::make_index_sequence<ArgsCount>());
	}
private:
	template<size_t N>
	static auto ArgAt(EvaluationStack& stack)
	{
		using arg_t = arg_info<0, N, TArgs...>;
		return reinterpret_cast<typename arg_t::Type&>(stack.GetFromTop(arg_t::Offset));
	}

	template<size_t ...ArgIdx>
	static auto CallImp(uintptr_t entryPoint, EvaluationStack& stack, std::index_sequence<ArgIdx...>)
	{
		auto func = reinterpret_cast<TRet(*)(TArgs...)>(entryPoint);
		func(ArgAt<ArgIdx>(stack)...);

		stack.Pop(ArgsSize);
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
