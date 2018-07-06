//
// Natsu CLR VM
//
#include <vm/EvaluationStack.hpp>

using namespace clr;
using namespace clr::vm;
using namespace clr::metadata;

void EvaluationStack::PushFrame(const MethodDesc* method, size_t argsSize, size_t argsCount, size_t retSize, uintptr_t* argsStore)
{
	if (argsCount)
	{
		auto argsBegin = stack_.end() - argsSize;
		std::copy(argsBegin, stack_.end(), argsStore);
		stack_.resize(stack_.size() - argsSize);

		stackType_.resize(stackType_.size() - argsCount);
	}

	auto offset = stack_.size();
	auto stackTypeOffset = stackType_.size();

	frames_.push({ method, offset, retSize, stackTypeOffset });
}

void CalleeInfo::BeginCall(const MethodDesc* method, EvaluationStack& stack)
{
	method_ = method;
	stack_ = &stack;

	data_.resize(method->ArgsSize + method->LocalVarsSize);
	stack.PushFrame(method, method->ArgsSize, method->ArgsCount, method->RetSize, data_.data());
}

void CalleeInfo::EndCall(bool hasRet)
{
	stack_->PopFrame(hasRet);
}

StackVar CalleeInfo::GetArg(size_t index)
{
	auto& info = method_->ArgsDesc[index];
	return { &data_.at(info.Offset), info.Type };
}

StackVar CalleeInfo::GetLocalVar(size_t index)
{
	auto& info = method_->LocalVarsDesc[index];
	return { &data_.at(method_->ArgsSize + info.Offset), info.Type };
}

size_t CalleeInfo::GetLocalVarSize(size_t index)
{
	auto offset = method_->LocalVarsDesc[index].Offset;
	return index + 1 == method_->LocalVarsCount ? method_->LocalVarsSize - offset : method_->LocalVarsDesc[index + 1].Offset - offset;
}
