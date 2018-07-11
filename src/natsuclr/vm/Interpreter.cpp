//
// Natsu CLR VM
//
#include <vm/Interpreter.hpp>
#include <utils.hpp>
#include <gc/gc.hpp>

using namespace clr;
using namespace clr::corlib;
using namespace clr::gc;
using namespace clr::vm;
using namespace clr::metadata;

Interpreter::Interpreter(loader::AssemblyLoader& assemblyLoader)
	:assemblyLoader_(assemblyLoader)
{

}

void Interpreter::ExecuteMethod(const MethodDesc& method)
{
	auto& calleeInfo = calleeInfo_.emplace();
	calleeInfo.BeginCall(&method, evalStack_);

	if (method.IsECall)
		method.ECall.Call(method.ECall.EntryPoint, calleeInfo);
	else
		ExecuteILMethod(method);

	calleeInfo.EndCall(true);
	calleeInfo_.pop();
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
void Interpreter::ExecuteOp<CEE_LDC_I4_1>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.Push(int32_t(1), ELEMENT_TYPE_I4);
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
	auto& calleeInfo = calleeInfo_.top();
	auto& info = calleeInfo.GetLocalVar(0);
	evalStack_.PushArgOrLocal(info.Data, calleeInfo.GetLocalVarSize(0), info.Type);
}

template<>
void Interpreter::ExecuteOp<CEE_STLOC_0>(OpInfo& op, OpArgsVal& args)
{
	auto& calleeInfo = calleeInfo_.top();
	auto& info = calleeInfo.GetLocalVar(0);
	evalStack_.PopArgOrLocal(info.Data, calleeInfo.GetLocalVarSize(0));
}

template<>
void Interpreter::ExecuteOp<CEE_LDARG_0>(OpInfo& op, OpArgsVal& args)
{
	auto& calleeInfo = calleeInfo_.top();
	auto& info = calleeInfo.GetArg(0);
	evalStack_.PushArgOrLocal(info.Data, calleeInfo.GetArgSize(0), info.Type);
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

template<>
void Interpreter::ExecuteOp<CEE_NEWOBJ>(OpInfo& op, OpArgsVal& args)
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

	auto obj = GC::Current().AllocateObject(method->Class);
	evalStack_.Push(obj, ELEMENT_TYPE_OBJECT);

	ExecuteMethod(*method);
	evalStack_.Push(obj, ELEMENT_TYPE_OBJECT);
}

template<>
void Interpreter::ExecuteOp<CEE_LDFLD>(OpInfo& op, OpArgsVal& args)
{
	mdToken token(static_cast<uint32_t>(args.i));
	const FieldDesc* field;

	switch (token.GetType())
	{
	case mdt_Field:
		field = &assemblyLoader_.GetField(token.As<mdt_Field>());
		break;
	default:
		assert(!"Invalid token type");
		break;
	}

	auto obj = evalStack_.Pop<ObjectRef<>>();
	auto offset = reinterpret_cast<uint8_t*>(obj.Value) + field->Offset;

	CorElementType type;

	evalStack_.PushBytes(offset, field->Size, field->Type);
}

template<>
void Interpreter::ExecuteOp<CEE_STFLD>(OpInfo& op, OpArgsVal& args)
{
	mdToken token(static_cast<uint32_t>(args.i));
	const FieldDesc* field;

	switch (token.GetType())
	{
	case mdt_Field:
		field = &assemblyLoader_.GetField(token.As<mdt_Field>());
		break;
	default:
		assert(!"Invalid token type");
		break;
	}

	auto objOff = align(field->Size, sizeof(uintptr_t)) / sizeof(uintptr_t) + 1;
	auto obj = ObjectRef<>(*evalStack_.GetFromTop(objOff));
	auto offset = reinterpret_cast<uint8_t*>(obj) + field->Offset;
	CorElementType type;

	evalStack_.PopBytes(offset, field->Size, type);
	evalStack_.Pop<ObjectRef<>>();
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
		EXECUTE_OP(CEE_LDC_I4_1);
		EXECUTE_OP(CEE_LDC_R4);
		EXECUTE_OP(CEE_LDC_R8);
		EXECUTE_OP(CEE_LDLOC_0);
		EXECUTE_OP(CEE_STLOC_0);
		EXECUTE_OP(CEE_LDARG_0);
		EXECUTE_OP(CEE_CONV_R4);
		EXECUTE_OP(CEE_BR_S);
		EXECUTE_OP(CEE_NEWOBJ);
		EXECUTE_OP(CEE_LDFLD);
		EXECUTE_OP(CEE_STFLD);
	default:
		assert(!"Invalid OpCode");
		break;
	}
}
