//
// Natsu CLR VM
//
#include <vm/Interpreter.hpp>
#include <utils.hpp>

using namespace clr;
using namespace clr::vm;
using namespace clr::metadata;

Interpreter::Interpreter(loader::AssemblyLoader& assemblyLoader)
	:assemblyLoader_(assemblyLoader)
{

}

void Interpreter::ExecuteMethod(const MethodDesc& method)
{
	evalStack_.PushFrame(&method, method.ArgsSize, method.RetSize);

	if (method.IsECall)
		method.ECall.Call(method.ECall.EntryPoint, evalStack_);
	else
		ExecuteILMethod(method);

	evalStack_.PopFrame();
}

void Interpreter::ExecuteILMethod(const MethodDesc& method)
{
	auto IP = method.BodyBegin;
	while (true)
	{
		OpInfo op(static_cast<OPCODE>(*IP));
		OpArgsVal opArgs;
		IP = op.fetch(IP, &opArgs);

		ExecuteOp(op, opArgs);

		switch (op.getFlow())
		{
		case FLOW_RETURN:
			return;
		default:
			break;
		}
	}
}

template<>
void Interpreter::ExecuteOp<CEE_NOP>(OpInfo& op, OpArgsVal& args)
{
}

template<>
void Interpreter::ExecuteOp<CEE_RET>(OpInfo& op, OpArgsVal& args)
{
}

template<>
void Interpreter::ExecuteOp<CEE_CALL>(OpInfo& op, OpArgsVal& args)
{
	mdToken token(static_cast<uint32_t>(args.i));
	const MethodDesc* method;

	switch (token.GetType())
	{
	case mdt_MethodDef:
		method = &assemblyLoader_.GetMethod(token.As<mdt_MethodDef>());
		break;
	default:
		assert(!"Invalid token type");
		break;
	}

	ExecuteMethod(*method);
}

template<>
void Interpreter::ExecuteOp<CEE_LDC_I4_S>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.Push(args.i);
}

template<>
void Interpreter::ExecuteOp<CEE_LDC_R4>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.Push(static_cast<float>(args.r));
}

#define EXECUTE_OP(opCode) case opCode: \
ExecuteOp<opCode>(op, args); \
break;

void Interpreter::ExecuteOp(OpInfo& op, OpArgsVal& args)
{
	switch (op.getOpcode())
	{
		EXECUTE_OP(CEE_NOP);
		EXECUTE_OP(CEE_RET);
		EXECUTE_OP(CEE_CALL);
		EXECUTE_OP(CEE_LDC_I4_S);
		EXECUTE_OP(CEE_LDC_R4);
	default:
		assert(!"Invalid OpCode");
		break;
	}
}
