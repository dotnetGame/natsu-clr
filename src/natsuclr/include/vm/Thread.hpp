//
// Natsu CLR VM
//
#pragma once
#include "EEClass.hpp"
#include "loader/AssemblyLoader.hpp"

namespace clr
{
	namespace vm
	{
		class ExecutionException : std::exception
		{
			using exception::exception;
		};

		class Thread
		{
		public:
			loader::AssemblyLoader* assemblyLoader_;

			void Execute(const MethodDesc& method);
		};
	}
}
