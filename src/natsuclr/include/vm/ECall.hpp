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

		struct ECall
		{
			const char* MethodName;
			size_t ParamsCount;
			uintptr_t EntryPoint;
		};

		struct ECallClass
		{
			const char* Namespace;
			const char* ClassName;
			const ECall* ECalls;
		};

		const ECall& FindECall(const MethodDesc& method);
	}
}
