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
		class Thread
		{
		public:
			loader::AssemblyLoader* assemblyLoader_;

			void Execute(const MethodDesc& method);
		};
	}
}
