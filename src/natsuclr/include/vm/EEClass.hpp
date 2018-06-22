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
		struct EEMethod
		{
			metadata::MDImporter* MDImporter;

			const char* Name;

			const uint8_t* Body;
		};

		struct EEClass
		{
			metadata::MDImporter* MDImporter;

			const char* TypeName;
			const char* TypeNamespace;

			EEMethod* FirstMethod;
			EEMethod* LastMethod;
		};
	}
}
