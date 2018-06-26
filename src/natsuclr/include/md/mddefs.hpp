//
// Natsu CLR Metadata
//
#pragma once
#include <cstdint>
#include <exception>

namespace clr
{
	namespace metadata
	{
		class BadMetadataException : std::exception
		{
			using exception::exception;
		};

		enum class AssemblyFlags : uint32_t
		{

		};

		enum class MethodImplAttributes : uint16_t
		{

		};

		enum class MethodAttributes : uint16_t
		{

		};

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

		enum class FieldAttributes : uint16_t
		{

		};

		enum class ParamAttributes : uint16_t
		{

		};

		enum ELEMENT_TYPE : uint8_t
		{
			ELEMENT_TYPE_END,
			ELEMENT_TYPE_VOID,
			ELEMENT_TYPE_BOOLEAN,
			ELEMENT_TYPE_CHAR,
			ELEMENT_TYPE_I1,
			ELEMENT_TYPE_U1,
			ELEMENT_TYPE_I2
		};

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
			mdt_AssemblyRef = 0x23,
			mdt_AssemblyRefOS = 0x25,
			mdt_AssemblyRefProcessor = 0x24,
			mdt_ClassLayout = 0x0F,
			mdt_Constant = 0x0B,
			mdt_CustomAttribute = 0x0C,
			mdt_DeclSecurity = 0x0E,
			mdt_EventMap = 0x12,
			mdt_Event = 0x14,
			mdt_ExportedType = 0x27,
			mdt_Field = 0x04,
			mdt_FieldLayout = 0x10,
			mdt_FieldMarshal = 0x0D,
			mdt_FieldRVA = 0x1D,
			mdt_File = 0x26,
			mdt_GenericParam = 0x2A,
			mdt_GenericParamConstraint = 0x2C,
			mdt_ImplMap = 0x1C,
			mdt_InterfaceImp = 0x09,
			mdt_ManifestResource = 0x28,
			mdt_MemberRef = 0x0A,
			mdt_MethodDef = 0x06,
			mdt_MethodImpl = 0x19,
			mdt_MethodSemantics = 0x18,
			mdt_MethodSpec = 0x2B,
			mdt_Module = 0x00,
			mdt_ModuleRef = 0x1A,
			mdt_NestedClass = 0x29,
			mdt_Param = 0x08,
			mdt_Property = 0x17,
			mdt_PropertyMap = 0x15,
			mdt_StandAloneSig = 0x11,
			mdt_TypeDef = 0x02,
			mdt_TypeRef = 0x01,
			mdt_TypeSpec = 0x1B,
			mdt_Count = 0x2d
		};

		enum CodedRowIndex
		{
			crid_TypeDefOrRef,
			crid_HasConstant,
			crid_HasCustomAttribute,
			crid_CustomAttributeType
		};

		template<MetadataTables Table>
		struct Ridx
		{
			uint32_t Index;

			uint32_t operator()() const noexcept { return Index; }
			uint32_t& operator()() noexcept { return Index; }
			operator bool() const noexcept { return Index != 0; }

			Ridx& operator++() noexcept
			{
				Index++;
				return *this;
			}

			bool operator==(const Ridx& other) const noexcept
			{
				return Index == other;
			}

			bool operator!=(const Ridx& other) const noexcept
			{
				return Index != other;
			}
		};

		template<StreamType Stream>
		struct Sidx
		{
			uint32_t Index;

			uint32_t operator()() const noexcept { return Index; }
			uint32_t& operator()() noexcept { return Index; }
		};

		template<CodedRowIndex Type>
		struct CodedRidx
		{

		};

		namespace impl
		{
			template<class T, T... Values>
			struct value_sequence
			{
				constexpr size_t size() const noexcept { return sizeof...(Values); }
			};

			template<size_t TagBits, MetadataTables... Types>
			struct CodedRidxImpl
			{
				using PackedTypes = value_sequence<MetadataTables, Types...>;

				static_assert(TagBits != 0, "Invalid tag bits.");
				static constexpr size_t SizeThreshold = 1 << (16 - TagBits);

				uint32_t CodedValue;

				size_t GetRidx() const noexcept { return CodedValue >> TagBits; }
				operator bool() const noexcept { return GetRidx() != 0; }
			protected:
				static constexpr size_t TagMask = (1 << TagBits) - 1;

				size_t GetTag() const noexcept { return CodedValue & TagMask; }
			};
		}

		template<>
		struct CodedRidx<crid_TypeDefOrRef> : public impl::CodedRidxImpl<2, mdt_TypeDef, mdt_TypeRef, mdt_TypeSpec>
		{
			MetadataTables GetType() const;
		};

		template<>
		struct CodedRidx<crid_HasConstant> : public impl::CodedRidxImpl<2, mdt_Field, mdt_Param, mdt_Property>
		{
			MetadataTables GetType() const;
		};

		template<>
		struct CodedRidx<crid_HasCustomAttribute> : public impl::CodedRidxImpl<5, mdt_MethodDef, mdt_Field, mdt_TypeRef, mdt_TypeDef, mdt_Param, mdt_InterfaceImp, mdt_MemberRef, mdt_Module, mdt_Property, mdt_Event, mdt_StandAloneSig, mdt_ModuleRef, mdt_TypeSpec, mdt_Assembly, mdt_AssemblyRef, mdt_File, mdt_ExportedType, mdt_ManifestResource, mdt_GenericParam, mdt_GenericParamConstraint, mdt_MethodSpec>
		{
			MetadataTables GetType() const;
		};

		template<>
		struct CodedRidx<crid_CustomAttributeType> : public impl::CodedRidxImpl<3, mdt_MethodDef, mdt_MemberRef>
		{
			MetadataTables GetType() const;
		};
	}
}
