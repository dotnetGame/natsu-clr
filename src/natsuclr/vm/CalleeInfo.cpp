//
// Natsu CLR VM
//
#include <cassert>
#include <utils.hpp>
#include <vm/EvaluationStack.hpp>

using namespace clr;
using namespace clr::vm;
using namespace clr::metadata;

CalleeInfo::CalleeInfo(const MethodDesc& method)
    : method_(method)
{
    size_t offset = 0;
    argPointer_ = offset;
    offset = align(offset + method.GetParamSize(), method.GetLocalVarAlign());
    localVarPointer_ = offset;
    offset = align(offset + method.GetLocalVarSize(), method.GetRetAlign());
    retPointer_ = offset;
    offset += method.GetRetSize();

    data_ = std::make_unique<uint8_t[]>(offset);
}

uint8_t* CalleeInfo::GetArg(size_t index, TypeDesc& type)
{
    assert(index < method_.ParamCount);
    auto& var = method_.ParamDescs[index];
    type = var.Type;
    return &data_[argPointer_ + var.Offset];
}

uint8_t* CalleeInfo::GetLocalVar(size_t index, TypeDesc& type)
{
    assert(index < method_.LocalVarCount);
    auto& var = method_.LocalVarDescs[index];
    type = var.Type;
    return &data_[localVarPointer_ + var.Offset];
}

uint8_t* CalleeInfo::GetRet(TypeDesc& type)
{
    type = method_.RetDesc;
    return &data_[retPointer_];
}
