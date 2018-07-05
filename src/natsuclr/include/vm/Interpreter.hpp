//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"
#include "../loader/AssemblyLoader.hpp"
#include "opinfo.h"
#include "EvaluationStack.hpp"

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
			void ExecuteILMethod(const MethodDesc& method);

			template<opcode_t OpCode>
			void ExecuteOp(OpInfo& op, OpArgsVal& args);
		private:
			EvaluationStack evalStack_;
			loader::AssemblyLoader& assemblyLoader_;
		};
	}
}
