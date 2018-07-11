//
// Natsu CLR Class Lib Native
//
#pragma once
#include "../../vm/EEClass.hpp"

namespace clr
{
	namespace corlib
	{
		struct ObjectHeader
		{
			const vm::EEClass* Class;
		};

		class Object
		{
		public:
			ObjectHeader & GetHeader() noexcept;
		};

		template<class T = Object>
		using ObjectRef = T * ;
	}
}
