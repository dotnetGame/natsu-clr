//
// Natsu CLR Metadata
//
#pragma once
#include <cstdint>
#include <exception>
#include "../utils.hpp"

namespace clr
{
namespace metadata
{
    class BadMetadataException : std::exception
    {
        using exception::exception;
    };

    class NotSupportedException : std::exception
    {
        using exception::exception;
    };

    enum class AssemblyHashAlgorithm : uint32_t
    {
    };

    enum class AssemblyFlags : uint32_t
    {

    };

    enum class EventFlags : uint32_t
    {
    };

    enum class FieldAttributes : uint16_t
    {
        CompilerControlled = 0x0,
        Private = 0x1,
        FamANDAssem = 0x2,
        Assembly = 0x3,
        Family = 0x4,
        FamORAssem = 0x5,
        Public = 0x6,
        Static = 0x10,
        InitOnly = 0x20,
        Literal = 0x40,
        NotSerialized = 0x80,
        SpecialName = 0x200,
        PInvokeImpl = 0x2000,
        RTSpecialName = 0x400,
        HasFieldMarshal = 0x1000,
        HasDefault = 0x8000,
        HasFieldRVA = 0x100
    };

#define FieldAttributes_FieldAccessMask 0x7

    enum class FileAttributes : uint32_t
    {
    };

    enum class GenericParamAttributes : uint16_t
    {
		None = 0x0000,
        Covariant = 0x0001,
        Contravariant = 0x0002,
        ReferenceTypeConstraint = 0x004,
        NotNullableValueTypeConstraint = 0x0008,
        DefaultConstructorConstraint = 0x0010
    };

#define GenericParamAttributes_VarianceMask 0x0003
#define GenericParamAttributes_SpecialConstraintMask 0x001C

    enum class MethodImplAttributes : uint16_t
    {
        IL = 0x0,
        Native = 0x1,
        OPTIL = 0x2,
        Runtime = 0x3,

        Unmanaged = 0x4,
        Managed = 0x0,

        InternalCall = 0x1000
    };

    enum class MethodAttributes : uint16_t
    {
        CompilerControlled = 0x0000,
        Private = 0x0001,
        FamANDAssem = 0x0002,
        Assem = 0x0003,
        Family = 0x0004,
        FamORAssem = 0x0005,
        Public = 0x0006,
        Static = 0x0010,
        Final = 0x0020,
        Virtual = 0x0040,
        HideBySig = 0x0080,
        ReuseSlot = 0x0000,
        NewSlot = 0x0100,
        Strict = 0x0200,
        Abstract = 0x0400,
        SpecialName = 0x0800,
        PInvokeImpl = 0x2000,
        UnmanagedExport = 0x0008,
        RTSpecialName = 0x1000,
        HasSecurity = 0x4000,
        RequireSecObject = 0x8000
    };

#define MethodAttributes_MemberAccessMask 0x0007
#define MethodAttributes_VtableLayoutMask 0x0010

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

    enum class ParamAttributes : uint16_t
    {

    };

    enum class PropertyAttributes : uint16_t
    {

    };

    enum class MethodSemanticsAttributes : uint16_t
    {

    };

#ifdef MAKE_ENUM_CLASS_BITMASK_TYPE
    MAKE_ENUM_CLASS_BITMASK_TYPE(MethodImplAttributes);
    MAKE_ENUM_CLASS_BITMASK_TYPE(FieldAttributes);
    MAKE_ENUM_CLASS_BITMASK_TYPE(MethodAttributes);
#endif

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
        crid_HasFieldMarshall,
        crid_HasDeclSecurity,
        crid_MemberRefParent,
        crid_HasSemantics,
        crid_MethodDefOrRef,
        crid_MemberForwarded,
        crid_Implementation,
        crid_CustomAttributeType,
        crid_ResolutionScope,
        crid_TypeOrMethodDef
    };

    template <MetadataTables Table>
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

    template <StreamType Stream>
    struct Sidx
    {
        uint32_t Index;

        uint32_t operator()() const noexcept { return Index; }
        uint32_t& operator()() noexcept { return Index; }
    };

    template <CodedRowIndex Type>
    struct CodedRidx
    {
    };

    namespace impl
    {
        template <class T, T... Values>
        struct value_sequence
        {
            constexpr size_t size() const noexcept { return sizeof...(Values); }
        };

        template <size_t TagBits, MetadataTables... Types>
        struct CodedRidxImpl
        {
            using PackedTypes = value_sequence<MetadataTables, Types...>;

            static_assert(TagBits != 0, "Invalid tag bits.");
            static constexpr size_t SizeThreshold = 1 << (16 - TagBits);

            uint32_t CodedValue;

            template <MetadataTables Table>
            Ridx<Table> As() const { return { CodedValue >> TagBits }; }
            operator bool() const noexcept { return (CodedValue >> TagBits) != 0; }

        protected:
            static constexpr size_t TagMask = (1 << TagBits) - 1;

            size_t GetTag() const noexcept { return CodedValue & TagMask; }
        };
    }

    template <>
    struct CodedRidx<crid_TypeDefOrRef> : public impl::CodedRidxImpl<2, mdt_TypeDef, mdt_TypeRef, mdt_TypeSpec>
    {
        MetadataTables GetType() const;
    };

    template <>
    struct CodedRidx<crid_HasConstant> : public impl::CodedRidxImpl<2, mdt_Field, mdt_Param, mdt_Property>
    {
        MetadataTables GetType() const;
    };

    template <>
    struct CodedRidx<crid_HasCustomAttribute> : public impl::CodedRidxImpl<5, mdt_MethodDef, mdt_Field, mdt_TypeRef, mdt_TypeDef, mdt_Param, mdt_InterfaceImp, mdt_MemberRef, mdt_Module, mdt_Property, mdt_Event, mdt_StandAloneSig, mdt_ModuleRef, mdt_TypeSpec, mdt_Assembly, mdt_AssemblyRef, mdt_File, mdt_ExportedType, mdt_ManifestResource, mdt_GenericParam, mdt_GenericParamConstraint, mdt_MethodSpec>
    {
        MetadataTables GetType() const;
    };

    template <>
    struct CodedRidx<crid_CustomAttributeType> : public impl::CodedRidxImpl<3, mdt_MethodDef, mdt_MemberRef>
    {
        MetadataTables GetType() const;
    };

    template <>
    struct CodedRidx<crid_HasSemantics> : public impl::CodedRidxImpl<1, mdt_Event, mdt_Property>
    {
        MetadataTables GetType() const;
    };

    template <>
    struct CodedRidx<crid_TypeOrMethodDef> : public impl::CodedRidxImpl<1, mdt_TypeDef, mdt_MethodDef>
    {
        MetadataTables GetType() const;
    };

    struct BlobData
    {
        const uint8_t* Data;
        size_t Length;
    };

    enum CorElementType : uint8_t
    {
        ELEMENT_TYPE_END = 0x00,
        ELEMENT_TYPE_VOID = 0x01,
        ELEMENT_TYPE_BOOLEAN = 0x02,
        ELEMENT_TYPE_CHAR = 0x03,
        ELEMENT_TYPE_I1 = 0x04,
        ELEMENT_TYPE_U1 = 0x05,
        ELEMENT_TYPE_I2 = 0x06,
        ELEMENT_TYPE_U2 = 0x07,
        ELEMENT_TYPE_I4 = 0x08,
        ELEMENT_TYPE_U4 = 0x09,
        ELEMENT_TYPE_I8 = 0x0a,
        ELEMENT_TYPE_U8 = 0x0b,
        ELEMENT_TYPE_R4 = 0x0c,
        ELEMENT_TYPE_R8 = 0x0d,
        ELEMENT_TYPE_STRING = 0x0e,

        // every type above PTR will be simple type
        ELEMENT_TYPE_PTR = 0x0f, // PTR <type>
        ELEMENT_TYPE_BYREF = 0x10, // BYREF <type>

        // Please use ELEMENT_TYPE_VALUETYPE. ELEMENT_TYPE_VALUECLASS is deprecated.
        ELEMENT_TYPE_VALUETYPE = 0x11, // VALUETYPE <class Token>
        ELEMENT_TYPE_CLASS = 0x12, // CLASS <class Token>
        ELEMENT_TYPE_VAR = 0x13, // a class type variable VAR <number>
        ELEMENT_TYPE_ARRAY = 0x14, // MDARRAY <type> <rank> <bcount> <bound1> ... <lbcount> <lb1> ...
        ELEMENT_TYPE_GENERICINST = 0x15, // GENERICINST <generic type> <argCnt> <arg1> ... <argn>
        ELEMENT_TYPE_TYPEDBYREF = 0x16, // TYPEDREF  (it takes no args) a typed referece to some other type

        ELEMENT_TYPE_I = 0x18, // native integer size
        ELEMENT_TYPE_U = 0x19, // native unsigned integer size
        ELEMENT_TYPE_FNPTR = 0x1b, // FNPTR <complete sig for the function including calling convention>
        ELEMENT_TYPE_OBJECT = 0x1c, // Shortcut for System.Object
        ELEMENT_TYPE_SZARRAY = 0x1d, // Shortcut for single dimension zero lower bound array
        // SZARRAY <type>
        ELEMENT_TYPE_MVAR = 0x1e, // a method type variable MVAR <number>

        // This is only for binding
        ELEMENT_TYPE_CMOD_REQD = 0x1f, // required C modifier : E_T_CMOD_REQD <mdTypeRef/mdTypeDef>
        ELEMENT_TYPE_CMOD_OPT = 0x20, // optional C modifier : E_T_CMOD_OPT <mdTypeRef/mdTypeDef>

        // This is for signatures generated internally (which will not be persisted in any way).
        ELEMENT_TYPE_INTERNAL = 0x21, // INTERNAL <typehandle>

        // Note that this is the max of base type excluding modifiers
        ELEMENT_TYPE_MAX = 0x22, // first invalid element type

        ELEMENT_TYPE_MODIFIER = 0x40,
        ELEMENT_TYPE_SENTINEL = 0x01 | ELEMENT_TYPE_MODIFIER, // sentinel for varargs
        ELEMENT_TYPE_PINNED = 0x05 | ELEMENT_TYPE_MODIFIER
    };
}
}
