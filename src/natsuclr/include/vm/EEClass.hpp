//
// Natsu CLR VM
//
#pragma once
#include "../md/mddefs.hpp"
#include <memory>
#include <string_view>
#include <vector>

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

    enum TypeDescAttributes
    {
        tda_Normal = 0x0,
        tda_ByRef = 0x1,
        tda_Generic = 0x2,
        tda_SzArray = 0x4
    };

#ifdef MAKE_ENUM_CLASS_BITMASK_TYPE
    MAKE_ENUM_CLASS_BITMASK_TYPE(TypeDescAttributes);
#endif

    struct EEClass;

    struct GenericDesc
    {
        uint32_t Number;
        std::string_view Name;
    };

    struct TypeDesc
    {
        TypeDescAttributes Flags;
        metadata::CorElementType Type;
        GenericDesc* GenericParam;

        EEClass* Class;

        bool IsEmpty() const noexcept;
        size_t GetStackSize() const noexcept;
        size_t GetAlign() const noexcept;
    };

    struct VarDesc
    {
        uint32_t Offset;
        TypeDesc Type;
    };

    class EvaluationStack;
    class CalleeInfo;
    struct EEClass;

    typedef void (*ECallInvoker)(uintptr_t entryPoint, CalleeInfo& callee);

    struct MethodDesc
    {
        EEClass* Class;

        std::string_view Name;
        bool IsECall;

        std::unique_ptr<VarDesc[]> ParamDescs;
        std::unique_ptr<VarDesc[]> LocalVarDescs;
        std::vector<GenericDesc> GenericParams;
        TypeDesc RetDesc;

        uint32_t ParamCount;
        uint32_t LocalVarSize;
        uint32_t LocalVarCount;

        union {
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

        size_t GetParamSize() const;
        size_t GetLocalVarSize() const;
        size_t GetRetSize() const;

        size_t GetParamAlign() const;
        size_t GetLocalVarAlign() const;
        size_t GetRetAlign() const;
    };

    struct FieldDesc
    {
        EEClass* Class;
        metadata::Ridx<metadata::mdt_Field> Ridx;
        metadata::FieldAttributes Flags;

        std::string_view Name;

        VarDesc Var;
    };

    struct EEClass
    {
        metadata::MDImporter* MDImporter;

        std::string_view TypeName;
        std::string_view TypeNamespace;

        EEClass* Parent;

        MethodDesc* FirstMethod;
        MethodDesc* LastMethod;

        FieldDesc* FirstField;
        FieldDesc* LastField;

        uint32_t Align;

        uint32_t InstanceSize;

        std::unique_ptr<uint8_t[]> StaticFields;
        std::vector<GenericDesc> GenericParams;

        ClassLoadLevel LoadLevel;
    };

    struct mdToken
    {
        mdToken(uint32_t value)
            : value_(value) {}

        static constexpr uint32_t TypeBitsOffset = 24;
        static constexpr uint32_t TypeMask = 0xFF << TypeBitsOffset;

        metadata::MetadataTables GetType() const noexcept { return static_cast<metadata::MetadataTables>((value_ & TypeMask) >> TypeBitsOffset); }

        template <metadata::MetadataTables Type>
        metadata::Ridx<Type> As() const noexcept { return { value_ & ~TypeMask }; }

        operator bool() const noexcept { return value_ != 0; }

    private:
        uint32_t value_;
    };
}
}
