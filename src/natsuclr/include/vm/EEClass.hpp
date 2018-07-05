//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"
#include <memory>

namespace clr
{
	namespace metadata
	{
		class MDImporter;
	}

	namespace vm
	{
		enum ClassLoadLevel
		{
			clsLoad_NotLoaded,
			clsLoad_InstanceFields,
			clsLoad_StaticFields
		};

		struct EEClass;
		struct MethodDesc
		{
			EEClass* Class;

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
					uintptr_t EntryPoint;
					size_t ParamsCount;
				} ECall;
			};
		};

		struct FieldDesc
		{
			EEClass* Class;
			metadata::Ridx<metadata::mdt_Field> Ridx;
			metadata::FieldAttributes Flags;

			const char* Name;

			uint32_t Offset;
			uint32_t Size;
		};

		struct EEClass
		{
			metadata::MDImporter* MDImporter;

			const char* TypeName;
			const char* TypeNamespace;

			MethodDesc* FirstMethod;
			MethodDesc* LastMethod;

			FieldDesc* FirstField;
			FieldDesc* LastField;

			uint32_t InstanceSize;
			uint32_t StaticSize;

			std::unique_ptr<uint8_t[]> StaticFields;

			ClassLoadLevel LoadLevel;
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
