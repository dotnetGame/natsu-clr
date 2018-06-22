//
// Natsu CLR VM
//
#pragma once
#include "EEClass.hpp"

namespace clr
{
	namespace vm
	{
		struct MethodTable
		{
			EEClass* Class;
		};
	}
}
