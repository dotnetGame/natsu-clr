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

		struct mdToken
		{
			mdToken(uint32_t value)
				:value_(value) {}

			static constexpr uint32_t TypeBitsOffset = 24;
			static constexpr uint32_t TypeMask = 0xFF << TypeBitsOffset;

			metadata::MetadataTables GetType() const noexcept { return static_cast<metadata::MetadataTables>((value_ & TypeMask) >> TypeBitsOffset); }

			template<metadata::MetadataTables Type>
			metadata::Ridx<Type> As() const noexcept { return {value_ & ~TypeMask }; }
		private:
			uint32_t value_;
		};
	}
}
