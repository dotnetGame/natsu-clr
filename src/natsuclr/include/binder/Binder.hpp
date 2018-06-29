//
// Natsu CLR Binder
//
#pragma once
#include "../loader/AssemblyLoader.hpp"

namespace clr
{
	class Binder
	{
	public:
		Binder(std::shared_ptr<loader::AssemblyLoader> assemblyLoader);

		vm::MethodDesc* BindMethod(const char* typeName, const char* methodName);
	private:
		std::shared_ptr<loader::AssemblyLoader> assemblyLoader_;
	};
}
