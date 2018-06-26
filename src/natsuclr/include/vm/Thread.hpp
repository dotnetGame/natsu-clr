//
// Natsu CLR VM
//
#pragma once
#include "EEClass.hpp"

namespace clr
{
	namespace vm
	{
		class Thread
		{
		public:
			void Execute(const EEMethod& method);
		};
	}
}
