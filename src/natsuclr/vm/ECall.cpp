//
// Natsu CLR VM
//
#include <vm/ECall.hpp>
#include <utils.hpp>
#include <cstring>
#include <classlibnative/System/Console.hpp>

using namespace clr;
using namespace clr::vm;

template<typename Func>
struct func_info;

template<typename TRet, typename... TArgs>
struct func_info<TRet(*)(TArgs...)>
{
	static const auto ArgsCount = sizeof...(TArgs);
};

#define ECFuncStart(funcvar, ns, cls) static const ECall funcvar[] = {
#define ECFuncElement(name, func) { name, func_info<decltype(func)>::ArgsCount, uintptr_t(func) },
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
