//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"
#include "../loader/AssemblyLoader.hpp"
#include "opinfo.h"
#include <stack>

namespace clr
{
	namespace vm
	{
		class ExecutionException : std::exception
		{
			using exception::exception;
		};

		class Interpreter
		{
		public:
			Interpreter(loader::AssemblyLoader& assemblyLoader);

			void ExecuteMethod(const MethodDesc& method);
		private:
			void ExecuteOp(OpInfo& op, OpArgsVal& args);

			template<opcode_t OpCode>
			void ExecuteOp(OpInfo& op, OpArgsVal& args);
		private:
			std::stack<uintptr_t> evalStack_;
			loader::AssemblyLoader& assemblyLoader_;
		};
	}
}
