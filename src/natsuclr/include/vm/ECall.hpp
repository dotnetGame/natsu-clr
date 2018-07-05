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
			const char* MethodName;

			uintptr_t EntryPoint;
			void(*Call)(uintptr_t entryPoint, EvaluationStack& stack);
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
