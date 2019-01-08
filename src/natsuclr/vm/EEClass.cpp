//
// Natsu CLR VM
//
#include <cassert>
#include <classlibnative/System/Object.hpp>
#include <vm/EEClass.hpp>

using namespace clr;
using namespace clr::corlib;
using namespace clr::metadata;
using namespace clr::vm;

bool TypeDesc::IsEmpty() const noexcept
{
    return Type == 0;
}

size_t TypeDesc::GetStackSize() const noexcept
{
    assert(!(Flags & tda_ByRef));

    assert(Type);

    switch (Type)
    {
    case ELEMENT_TYPE_VOID:
        return 0;
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
        return 1;
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
        return 2;
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
        return 4;
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
        return 8;
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_OBJECT:
        return sizeof(ObjectRef<>);
    case ELEMENT_TYPE_VALUETYPE:
        assert(Class && Class->LoadLevel >= clsLoad_InstanceFields);
        return Class->InstanceSize;
    case ELEMENT_TYPE_I:
        return sizeof(intptr_t);
    case ELEMENT_TYPE_U:
        return sizeof(uintptr_t);
    case ELEMENT_TYPE_VAR:
        return 0;
    default:
        assert(!"Invalid type.");
        return 0;
    }
}

size_t TypeDesc::GetAlign() const noexcept
{
    assert(!(Flags & tda_ByRef));

    assert(Type);

    switch (Type)
    {
    case ELEMENT_TYPE_VOID:
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
        return 1;
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
        return 2;
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
        return 4;
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
        return 8;
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_OBJECT:
        return sizeof(ObjectRef<>);
    case ELEMENT_TYPE_VALUETYPE:
        assert(Class && Class->LoadLevel >= clsLoad_InstanceFields);
        return Class->Align;
    case ELEMENT_TYPE_I:
        return sizeof(intptr_t);
    case ELEMENT_TYPE_U:
        return sizeof(uintptr_t);
    case ELEMENT_TYPE_VAR:
        return 1;
    default:
        assert(!"Invalid type.");
        return 1;
    }
}

size_t MethodDesc::GetParamSize() const
{
    if (ParamCount)
    {
        auto& desc = ParamDescs[ParamCount - 1];
        return desc.Offset + desc.Type.GetStackSize();
    }

    return 0;
}

size_t MethodDesc::GetLocalVarSize() const
{
    if (LocalVarCount)
    {
        auto& desc = LocalVarDescs[LocalVarCount - 1];
        return desc.Offset + desc.Type.GetStackSize();
    }

    return 0;
}

size_t MethodDesc::GetRetSize() const
{
    return RetDesc.GetStackSize();
}

size_t MethodDesc::GetParamAlign() const
{
    if (ParamCount)
        return ParamDescs[0].Type.GetAlign();
    return 1;
}

size_t MethodDesc::GetLocalVarAlign() const
{
    if (LocalVarCount)
        return LocalVarDescs[0].Type.GetAlign();
    return 1;
}

size_t MethodDesc::GetRetAlign() const
{
    return RetDesc.GetAlign();
}
