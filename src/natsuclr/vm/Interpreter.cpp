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
	calleeInfo_.BeginCall(&method, evalStack_);

	if (method.IsECall)
		method.ECall.Call(method.ECall.EntryPoint, calleeInfo_);
	else
		ExecuteILMethod(method);

	calleeInfo_.EndCall(true);
}

void Interpreter::ExecuteILMethod(const MethodDesc& method)
{
	IP_.push(method.BodyBegin);
	while (true)
	{
		OpInfo op(static_cast<OPCODE>(*IP_.top()));
		OpArgsVal opArgs;
		IP_.top() = op.fetch(IP_.top(), &opArgs);

		ExecuteOp(op, opArgs);

		switch (op.getFlow())
		{
		case FLOW_RETURN:
			IP_.pop();
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
	evalStack_.Push(args.i, ELEMENT_TYPE_I4);
}

template<>
void Interpreter::ExecuteOp<CEE_LDC_R4>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.Push(static_cast<float>(args.r), ELEMENT_TYPE_R4);
}

template<>
void Interpreter::ExecuteOp<CEE_LDC_R8>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.Push(args.r, ELEMENT_TYPE_R8);
}

template<>
void Interpreter::ExecuteOp<CEE_LDLOC_0>(OpInfo& op, OpArgsVal& args)
{
	auto& info = calleeInfo_.GetLocalVar(0);
	evalStack_.PushVar(reinterpret_cast<const uint8_t*>(info.Data), calleeInfo_.GetLocalVarSize(0), info.Type);
}

template<>
void Interpreter::ExecuteOp<CEE_STLOC_0>(OpInfo& op, OpArgsVal& args)
{
	auto& info = calleeInfo_.GetLocalVar(0);
	evalStack_.PopVar(reinterpret_cast<uint8_t*>(info.Data), calleeInfo_.GetLocalVarSize(0));
}

template<>
void Interpreter::ExecuteOp<CEE_CONV_R4>(OpInfo& op, OpArgsVal& args)
{
	float value;
	switch (evalStack_.GetTopType())
	{
	case ELEMENT_TYPE_R8:
		value = static_cast<float>(evalStack_.Pop<double>().Value);
		break;
	default:
		THROW_ALWAYS(NotSupportedException);
		break;
	}

	evalStack_.Push(value, ELEMENT_TYPE_R4);
}

template<>
void Interpreter::ExecuteOp<CEE_BR_S>(OpInfo& op, OpArgsVal& args)
{
	IP_.top() += args.i;
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
		EXECUTE_OP(CEE_LDC_R8);
		EXECUTE_OP(CEE_LDLOC_0);
		EXECUTE_OP(CEE_STLOC_0);
		EXECUTE_OP(CEE_CONV_R4);
		EXECUTE_OP(CEE_BR_S);
	default:
		assert(!"Invalid OpCode");
		break;
	}
}
