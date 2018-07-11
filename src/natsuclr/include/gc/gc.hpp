//
// Natsu CLR GC
//
#pragma once
#include "../classlibnative/System/Object.hpp"

namespace clr
{
	namespace gc
	{
		class GC
		{
		public:
			static GC& Current();

			corlib::ObjectRef<> AllocateObject(const vm::EEClass* cls);
		private:
			GC();
		};
	}
}
