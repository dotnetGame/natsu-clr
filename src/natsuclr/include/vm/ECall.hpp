//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"

namespace clr
{
	namespace vm
	{
		struct ECall
		{
			const char* Namespace;
			const char* ClassName;
			const char* MethodName;
		};
	}
}
