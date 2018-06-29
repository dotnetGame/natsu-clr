//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"

namespace clr
{
	namespace metadata
	{
		class MDImporter;
	}

	namespace vm
	{
		struct MethodDesc
		{
			metadata::MDImporter* MDImporter;

			const char* Name;
			bool IsECall;

			union
			{
				struct
				{
					uint16_t MaxStack;
					const uint8_t* BodyBegin;
					const uint8_t* BodyEnd;
				};

				struct
				{
					uintptr_t ECallEntry;
				};
			};
		};

		struct EEClass
		{
			metadata::MDImporter* MDImporter;

			const char* TypeName;
			const char* TypeNamespace;

			MethodDesc* FirstMethod;
			MethodDesc* LastMethod;
		};
	}
}
