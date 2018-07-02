//
// Natsu CLR VM
//
#include <vm/Thread.hpp>
#include <vm/opinfo.h>

using namespace clr;
using namespace clr::vm;
using namespace clr::metadata;

struct InterpretterContext
{
	loader::AssemblyLoader* assemblyLoader_;
	MDImporter* mdImporter_;

	void ExecuteMethod(const MethodDesc& method);
	void ExecuteOp(OpInfo& op, OpArgsVal& args);

	template<opcode_t OpCode>
	void ExecuteOp(OpInfo& op, OpArgsVal& args);
};

template<>
void InterpretterContext::ExecuteOp<CEE_NOP>(OpInfo& op, OpArgsVal& args)
{
}

template<>
void InterpretterContext::ExecuteOp<CEE_RET>(OpInfo& op, OpArgsVal& args)
{
}

template<>
void InterpretterContext::ExecuteOp<CEE_CALL>(OpInfo& op, OpArgsVal& args)
{
	mdToken token(static_cast<uint32_t>(args.i));
	const MethodDesc* method;

	switch (token.GetType())
	{
	case mdt_MethodDef:
		method = &assemblyLoader_->GetMethod(token.As<mdt_MethodDef>());
		break;
	default:
		assert(!"Invalid token type");
		break;
	}

	ExecuteMethod(*method);
}

#define EXECUTE_OP(opCode) case opCode: \
ExecuteOp<opCode>(op, args); \
break;

void InterpretterContext::ExecuteOp(OpInfo& op, OpArgsVal& args)
{
	switch (op.getOpcode())
	{
		EXECUTE_OP(CEE_NOP);
		EXECUTE_OP(CEE_RET);
		EXECUTE_OP(CEE_CALL);
	default:
		assert(!"Invalid OpCode");
		break;
	}
}

void InterpretterContext::ExecuteMethod(const MethodDesc& method)
{
	if (method.IsECall)
	{

	}
	else
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
}

void Thread::Execute(const MethodDesc& method)
{
	InterpretterContext context;
	context.mdImporter_ = method.MDImporter;
	context.assemblyLoader_ = assemblyLoader_;

	context.ExecuteMethod(method);
}
