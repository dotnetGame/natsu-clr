//
// Natsu CLR VM
//
#include <cassert>
#include <gc/gc.hpp>
#include <utils.hpp>
#include <vm/EvaluationStack.hpp>

using namespace clr;
using namespace clr::corlib;
using namespace clr::gc;
using namespace clr::vm;
using namespace clr::metadata;

EvaluationStack::EvaluationStack(size_t stackSize)
    : stackSize_(stackSize), stack_(std::make_unique<uint8_t[]>(stackSize)), stackPointer_(0)
{
}

void EvaluationStack::PushVar(const void* value, const TypeDesc& type)
{
    auto size = type.GetStackSize();
    auto offset = align(stackPointer_, type.GetAlign());
    THROW_IF_NOT(offset + size <= stackSize_, StackOverflowException);

    VarDesc var { (uint32_t)offset, type };
    stackVars_.emplace_back(var);
    memcpy(&stack_[offset], value, size);
    stackPointer_ += offset + size;
}

uint8_t* EvaluationStack::GetTopVar(TypeDesc& type)
{
    assert(!stackVars_.empty());
    auto& top = stackVars_.back();
    type = top.Type;
    return &stack_[top.Offset];
}

void EvaluationStack::PopVar()
{
    assert(!stackVars_.empty());
    stackVars_.pop_back();

    if (stackVars_.empty())
    {
        stackPointer_ = 0;
    }
    else
    {
        auto& top = stackVars_.back();
        stackPointer_ = top.Offset + top.Type.GetStackSize();
    }
}

uint8_t* EvaluationStack::PopTopVar(TypeDesc& type)
{
    auto data = GetTopVar(type);
    PopVar();
    return data;
}

void EvaluationStack::PushFrame(const MethodDesc& method)
{
    FrameMarker marker { method, stackVars_.size() };
    frames_.emplace(marker);
}

void EvaluationStack::PopFrame()
{
    stackVars_.resize(frames_.top().VarsOffset);

    if (stackVars_.empty())
    {
        stackPointer_ = 0;
    }
    else
    {
        auto& top = stackVars_.back();
        stackPointer_ = top.Offset + top.Type.GetStackSize();
    }
}
