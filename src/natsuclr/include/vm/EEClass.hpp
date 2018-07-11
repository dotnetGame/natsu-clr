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

		struct VarDesc
		{
			uint32_t Offset;
			metadata::CorElementType Type;

			VarDesc() = default;
			VarDesc(uint32_t offset, metadata::CorElementType type)
				:Offset(offset), Type(type) {}
		};

		class EvaluationStack;
		class CalleeInfo;
		struct EEClass;

		typedef void(*ECallInvoker)(uintptr_t entryPoint, CalleeInfo& callee);

		struct MethodDesc
		{
			EEClass* Class;

			const char* Name;
			bool IsECall;

			uint32_t ArgsSize;
			uint32_t ArgsCount;
			uint32_t LocalVarsSize;
			uint32_t LocalVarsCount;
			uint32_t RetSize;

			std::unique_ptr<VarDesc[]> ArgsDesc;
			std::unique_ptr<VarDesc[]> LocalVarsDesc;

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
					ECallInvoker Call;
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

			metadata::CorElementType Type;
		};

		struct EEClass
		{
			metadata::MDImporter* MDImporter;

			const char* TypeName;
			const char* TypeNamespace;

			EEClass* Parent;

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

			operator bool() const noexcept { return value_ != 0; }
		private:
			uint32_t value_;
		};
	}
}
