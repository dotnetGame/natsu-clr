//
// Natsu CLR Metadata
//
#pragma once
#include <cstdint>

namespace clr
{
	namespace metadata
	{
		enum class TypeAttributes : uint32_t
		{
			// Visibility attributes
			NotPublic = 0x0,
			Public = 0x1,
			NestedPublic = 0x2,
			NestedPrivate = 0x3,
			NestedFamily = 0x4,
			NestedAssembly = 0x5,
			NestedFamANDAssem = 0x6,
			NestedFamORAssem = 0x7
		};

#define TypeAttributes_VisibilityMask 0x7

		enum StreamType
		{
			stm_String = 1,
			stm_GUID = 2,
			stm_Blob = 4
		};

		enum MetadataTables
		{
			mdt_Assembly = 0x20,
			mdt_AssemblyOS = 0x22,
			mdt_AssemblyProcessor = 0x21,
			mdt_MethodDef = 0x06,
			mdt_Module = 0x00,
			mdt_TypeDef = 0x02,
			mdt_Count = 0x2d
		};

		template<MetadataTables Table>
		struct Ridx
		{
			uint32_t Index;

			uint32_t operator()() const noexcept { return Index; }
			uint32_t& operator()() noexcept { return Index; }
		};

		template<StreamType Stream>
		struct Sidx
		{
			uint32_t Index;

			uint32_t operator()() const noexcept { return Index; }
			uint32_t& operator()() noexcept { return Index; }
		};

		struct TypeDefOrRef
		{
			uint32_t CodedValue;

			MetadataTables GetType() const noexcept;
		};
	}
}
