//
// Natsu CLR VM
//
#include <gc/gc.hpp>
#include <utils.hpp>
#include <vm/Interpreter.hpp>

using namespace clr;
using namespace clr::corlib;
using namespace clr::gc;
using namespace clr::vm;
using namespace clr::metadata;

Interpreter::Interpreter(loader::AssemblyLoader& assemblyLoader)
    : assemblyLoader_(assemblyLoader)
{
}

class CallHelper
{
public:
    CallHelper(const MethodDesc& method, CalleeInfo& calleeInfo, EvaluationStack& stack)
        : method_(method), calleeInfo_(calleeInfo), stack_(stack)
    {
    }

    void BeginCall()
    {
        for (int32_t i = (int32_t)method_.ParamCount - 1; i >= 0; i--)
        {
            TypeDesc srcType, destType;
            auto src = stack_.GetTopVar(srcType);
            auto dest = calleeInfo_.GetArg(i, destType);
            assert(srcType.GetStackSize() == destType.GetStackSize());
            memcpy(dest, src, srcType.GetStackSize());
            stack_.PopVar();
        }

        stack_.PushFrame(method_);
    }

    void EndCall(bool hasRet)
    {
        stack_.PopFrame();

        if (hasRet)
        {
            TypeDesc srcType;
            auto src = calleeInfo_.GetRet(srcType);
            stack_.PushVar(src, srcType);
        }
    }

private:
    const MethodDesc& method_;
    CalleeInfo& calleeInfo_;
    EvaluationStack& stack_;
};

void Interpreter::ExecuteMethod(const MethodDesc& method)
{
    auto& calleeInfo = calleeInfo_.emplace(method);
    CallHelper callHelper(method, calleeInfo, evalStack_);
    callHelper.BeginCall();

    if (method.IsECall)
        method.ECall.Call(method.ECall.EntryPoint, calleeInfo);
    else
        ExecuteILMethod(method);

    callHelper.EndCall(true);
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

template <>
void Interpreter::ExecuteOp<CEE_NOP>(OpInfo& op, OpArgsVal& args)
{
}

template <>
void Interpreter::ExecuteOp<CEE_RET>(OpInfo& op, OpArgsVal& args)
{
}

template <>
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

template <>
void Interpreter::ExecuteOp<CEE_LDC_I4_S>(OpInfo& op, OpArgsVal& args)
{
    evalStack_.PushVar(&args.i, { false, ELEMENT_TYPE_I4, nullptr });
}

template <>
void Interpreter::ExecuteOp<CEE_LDC_I4_1>(OpInfo& op, OpArgsVal& args)
{
    int32_t i = 1;
    evalStack_.PushVar(&i, { false, ELEMENT_TYPE_I4, nullptr });
}

template <>
void Interpreter::ExecuteOp<CEE_LDC_R4>(OpInfo& op, OpArgsVal& args)
{
    auto f = static_cast<float>(args.r);
    evalStack_.PushVar(&f, { false, ELEMENT_TYPE_R4, nullptr });
}

template <>
void Interpreter::ExecuteOp<CEE_LDC_R8>(OpInfo& op, OpArgsVal& args)
{
    evalStack_.PushVar(&args.r, { false, ELEMENT_TYPE_R8, nullptr });
}

template <>
void Interpreter::ExecuteOp<CEE_LDLOC_0>(OpInfo& op, OpArgsVal& args)
{
    TypeDesc srcType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = calleeInfo.GetLocalVar(0, srcType);
    evalStack_.PushVar(src, srcType);
}

template <>
void Interpreter::ExecuteOp<CEE_STLOC_0>(OpInfo& op, OpArgsVal& args)
{
    TypeDesc srcType, destType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = evalStack_.GetTopVar(srcType);
    auto dest = calleeInfo.GetLocalVar(0, destType);
    assert(srcType.GetStackSize() == destType.GetStackSize());
    memcpy(dest, src, srcType.GetStackSize());
    evalStack_.PopVar();
}

template <>
void Interpreter::ExecuteOp<CEE_LDARG_0>(OpInfo& op, OpArgsVal& args)
{
    TypeDesc srcType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = calleeInfo.GetArg(0, srcType);
    evalStack_.PushVar(src, srcType);
}

template <>
void Interpreter::ExecuteOp<CEE_CONV_R4>(OpInfo& op, OpArgsVal& args)
{
    float value;
    TypeDesc srcType;
    auto src = evalStack_.GetTopVar(srcType);
    switch (srcType.Type)
    {
    case ELEMENT_TYPE_R8:
        value = static_cast<float>(*reinterpret_cast<double*>(src));
        break;
    default:
        THROW_ALWAYS(NotSupportedException);
        break;
    }

    evalStack_.PopVar();
    evalStack_.PushVar(&value, { false, ELEMENT_TYPE_R4, nullptr });
}

template <>
void Interpreter::ExecuteOp<CEE_BR_S>(OpInfo& op, OpArgsVal& args)
{
    IP_.top() += args.i;
}

template <>
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
    evalStack_.PushVar(&obj, { false, ELEMENT_TYPE_CLASS, method->Class });

    ExecuteMethod(*method);
    evalStack_.PushVar(&obj, { false, ELEMENT_TYPE_CLASS, method->Class });
}

template <>
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

    TypeDesc objType;
    auto obj = reinterpret_cast<ObjectRef<>*>(evalStack_.GetTopVar(objType));
    auto offset = reinterpret_cast<uint8_t*>(obj->Get()) + field->Var.Offset;

    evalStack_.PopVar();
    evalStack_.PushVar(offset, field->Var.Type);
}

template <>
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

    TypeDesc objType, srcType;
    auto src = evalStack_.GetTopVar(srcType);
    evalStack_.PopVar();

    auto obj = reinterpret_cast<ObjectRef<>*>(evalStack_.GetTopVar(objType));
    auto dest = reinterpret_cast<uint8_t*>(obj->Get()) + field->Var.Offset;
    assert(srcType.GetStackSize() == field->Var.Type.GetStackSize());
    memcpy(dest, src, srcType.GetStackSize());
    evalStack_.PopVar();
}

#define EXECUTE_OP(opCode)           \
    case opCode:                     \
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
