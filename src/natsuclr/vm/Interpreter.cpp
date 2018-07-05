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

template<typename T, size_t N>
struct ECallParam
{
	using Type = T;
};

template<typename Seq>
struct ECallThunkImpl;

template<size_t ...Idx>
struct ECallThunkImpl<std::index_sequence<Idx...>>
{
	typedef uintptr_t(*Callable)(typename ECallParam<uintptr_t, Idx>::Type...);
};

template<size_t ParamsCount>
struct ECallThunk : ECallThunkImpl<std::make_index_sequence<ParamsCount>>
{
	std::stack<uintptr_t>& EvalStack;

	ECallThunk(std::stack<uintptr_t>& evalStack)
		:EvalStack(evalStack)
	{

	}

	uintptr_t operator()(uintptr_t entryPoint)
	{
		return Invoke(entryPoint, std::make_index_sequence<ParamsCount>());
	}
private:
	uintptr_t PopImpl()
	{
		auto value = EvalStack.top();
		EvalStack.pop();
		return value;
	}

	template<size_t Idx>
	uintptr_t Pop()
	{
		return PopImpl();
	}

	template<size_t ...Idx>
	uintptr_t Invoke(uintptr_t entryPoint, std::index_sequence<Idx...>)
	{
		auto func = reinterpret_cast<Callable>(entryPoint);
		return func(Pop<Idx>()...);
	}
};

#define DEFINE_ECALL_THUNK(n) \
case n: { ECallThunk<n> thunk(evalStack_); thunk(method.ECall.EntryPoint); break; }

void Interpreter::ExecuteMethod(const MethodDesc& method)
{
	if (method.IsECall)
	{
		switch (method.ECall.ParamsCount)
		{
			DEFINE_ECALL_THUNK(0);
			DEFINE_ECALL_THUNK(1);
			DEFINE_ECALL_THUNK(2);
			DEFINE_ECALL_THUNK(3);
			DEFINE_ECALL_THUNK(4);
			DEFINE_ECALL_THUNK(5);
		default:
			THROW_ALWAYS(ExecutionException, "ECall params count exceeded.");
		}
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
	evalStack_.push(args.i);
}

template<>
void Interpreter::ExecuteOp<CEE_LDC_R4>(OpInfo& op, OpArgsVal& args)
{
	evalStack_.push(args.r);
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
