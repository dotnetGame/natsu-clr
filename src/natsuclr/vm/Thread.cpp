//
// Natsu CLR VM
//
#include <vm/Thread.hpp>
#include <vm/opinfo.h>

using namespace clr;
using namespace clr::vm;

void ExecuteOp(OpInfo& op)
{
	switch (op.getOpcode())
	{
	case CEE_NOP:
		break;
	case CEE_RET:
		break;
	default:
		assert(!"Invalid OpCode");
		break;
	}
}

void ExecuteMethod(const EEMethod& method)
{
	auto IP = method.BodyBegin;
	while (true)
	{
		OpInfo op(static_cast<OPCODE>(*IP));
		OpArgsVal opArgs;
		IP = op.fetch(IP, &opArgs);

		ExecuteOp(op);

		switch (op.getFlow())
		{
		case FLOW_RETURN:
			return;
		default:
			break;
		}
	}
}

void Thread::Execute(const EEMethod& method)
{
	ExecuteMethod(method);
}
