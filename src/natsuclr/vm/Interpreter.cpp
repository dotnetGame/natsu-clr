//
// Natsu CLR VM
//
#include <binder/CorlibBinder.hpp>
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
            if (srcType.Type != ELEMENT_TYPE_VOID)
                stack_.PushVar(src, srcType);
        }
    }

private:
    const MethodDesc& method_;
    CalleeInfo& calleeInfo_;
    EvaluationStack& stack_;
};

TypeDesc ClassToType(EEClass* cls)
{
    CorElementType type;
    auto& types = CorlibBinder::Current().Types;
    if (cls == types.Boolean)
        type = ELEMENT_TYPE_BOOLEAN;
    else if (cls == types.Double)
        type = ELEMENT_TYPE_R8;
    else if (cls == types.Int16)
        type = ELEMENT_TYPE_I2;
    else if (cls == types.Int32)
        type = ELEMENT_TYPE_I4;
    else if (cls == types.Object)
        type = ELEMENT_TYPE_OBJECT;
    else if (cls == types.Single)
        type = ELEMENT_TYPE_R4;
    else if (cls == types.UInt32)
        type = ELEMENT_TYPE_U4;
    else if (cls == types.Void)
        type = ELEMENT_TYPE_VOID;
    else
        type = ELEMENT_TYPE_CLASS;
    return { tda_Normal, type, 0, cls };
}

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

#define IMP_LDARG(op, val)                                         \
    template <>                                                    \
    void Interpreter::ExecuteOp<op>(OpInfo & op, OpArgsVal & args) \
    {                                                              \
        ExecuteOp_LDARG(static_cast<size_t>(val));                 \
    }

IMP_LDARG(CEE_LDARG, args.i)
IMP_LDARG(CEE_LDARG_S, args.i)
IMP_LDARG(CEE_LDARG_0, 0)
IMP_LDARG(CEE_LDARG_1, 1)
IMP_LDARG(CEE_LDARG_2, 2)
IMP_LDARG(CEE_LDARG_3, 3)

#define IMP_LDLOC(op, val)                                         \
    template <>                                                    \
    void Interpreter::ExecuteOp<op>(OpInfo & op, OpArgsVal & args) \
    {                                                              \
        ExecuteOp_LDLOC(static_cast<size_t>(val));                 \
    }

IMP_LDLOC(CEE_LDLOC, args.i)
IMP_LDLOC(CEE_LDLOC_S, args.i)
IMP_LDLOC(CEE_LDLOC_0, 0)
IMP_LDLOC(CEE_LDLOC_1, 1)
IMP_LDLOC(CEE_LDLOC_2, 2)
IMP_LDLOC(CEE_LDLOC_3, 3)

#define IMP_STLOC(op, val)                                         \
    template <>                                                    \
    void Interpreter::ExecuteOp<op>(OpInfo & op, OpArgsVal & args) \
    {                                                              \
        ExecuteOp_STLOC(static_cast<size_t>(val));                 \
    }

IMP_STLOC(CEE_STLOC, args.i)
IMP_STLOC(CEE_STLOC_S, args.i)
IMP_STLOC(CEE_STLOC_0, 0)
IMP_STLOC(CEE_STLOC_1, 1)
IMP_STLOC(CEE_STLOC_2, 2)
IMP_STLOC(CEE_STLOC_3, 3)

#define IMP_LDC_I4(op, val)                                        \
    template <>                                                    \
    void Interpreter::ExecuteOp<op>(OpInfo & op, OpArgsVal & args) \
    {                                                              \
        ExecuteOp_LDC_I4(static_cast<size_t>(val));                \
    }

IMP_LDC_I4(CEE_LDC_I4, args.i)
IMP_LDC_I4(CEE_LDC_I4_S, args.i)
IMP_LDC_I4(CEE_LDC_I4_0, 0)
IMP_LDC_I4(CEE_LDC_I4_1, 1)
IMP_LDC_I4(CEE_LDC_I4_2, 2)
IMP_LDC_I4(CEE_LDC_I4_3, 3)
IMP_LDC_I4(CEE_LDC_I4_4, 4)
IMP_LDC_I4(CEE_LDC_I4_5, 5)
IMP_LDC_I4(CEE_LDC_I4_6, 6)
IMP_LDC_I4(CEE_LDC_I4_7, 7)
IMP_LDC_I4(CEE_LDC_I4_8, 8)

template <>
void Interpreter::ExecuteOp<CEE_LDC_R4>(OpInfo& op, OpArgsVal& args)
{
    auto f = static_cast<float>(args.r);
    evalStack_.PushVar(&f, { tda_Normal, ELEMENT_TYPE_R4, 0, CorlibBinder::Current().Types.Single });
}

template <>
void Interpreter::ExecuteOp<CEE_LDC_R8>(OpInfo& op, OpArgsVal& args)
{
    evalStack_.PushVar(&args.r, { tda_Normal, ELEMENT_TYPE_R8, 0, CorlibBinder::Current().Types.Double });
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
    evalStack_.PushVar(&value, { tda_Normal, ELEMENT_TYPE_R4, 0, CorlibBinder::Current().Types.Single });
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

    auto type = ClassToType(method->Class);

    auto obj = GC::Current().AllocateObject(type.Class);
    evalStack_.PushVar(&obj, type);

    ExecuteMethod(*method);
    evalStack_.PushVar(&obj, type);
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

template <>
void Interpreter::ExecuteOp<CEE_NEWARR>(OpInfo& op, OpArgsVal& args)
{
    mdToken token(static_cast<uint32_t>(args.i));
    EEClass* cls;

    switch (token.GetType())
    {
    case mdt_TypeDef:
        cls = &assemblyLoader_.GetClass(token.As<mdt_TypeDef>());
        break;
    default:
        assert(!"Invalid token type");
        break;
    }

    auto obj = GC::Current().AllocateArray(cls, PopIndexOrArrayLength());
    evalStack_.PushVar(&obj, { tda_Normal | tda_SzArray, ELEMENT_TYPE_SZARRAY, 0, cls });
}

template <>
void Interpreter::ExecuteOp<CEE_DUP>(OpInfo& op, OpArgsVal& args)
{
    TypeDesc srcType;
    auto src = reinterpret_cast<ObjectRef<>*>(evalStack_.GetTopVar(srcType));
    evalStack_.PushVar(src, srcType);
}

#define IMP_STELEM(op, type)                                       \
    template <>                                                    \
    void Interpreter::ExecuteOp<op>(OpInfo & op, OpArgsVal & args) \
    {                                                              \
        ExecuteOp_STELEM(type);                                    \
    }

IMP_STELEM(CEE_STELEM_I1, ELEMENT_TYPE_I1)
IMP_STELEM(CEE_STELEM_I2, ELEMENT_TYPE_I2)
IMP_STELEM(CEE_STELEM_I4, ELEMENT_TYPE_I4)
IMP_STELEM(CEE_STELEM_I8, ELEMENT_TYPE_I8)
IMP_STELEM(CEE_STELEM_R4, ELEMENT_TYPE_R4)
IMP_STELEM(CEE_STELEM_R8, ELEMENT_TYPE_R8)
IMP_STELEM(CEE_STELEM_I, ELEMENT_TYPE_I)
IMP_STELEM(CEE_STELEM_REF, ELEMENT_TYPE_BYREF)

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
        EXECUTE_OP(CEE_LDARG);
        EXECUTE_OP(CEE_LDARG_S);
        EXECUTE_OP(CEE_LDARG_0);
        EXECUTE_OP(CEE_LDARG_1);
        EXECUTE_OP(CEE_LDARG_2);
        EXECUTE_OP(CEE_LDARG_3);
        EXECUTE_OP(CEE_LDLOC);
        EXECUTE_OP(CEE_LDLOC_S);
        EXECUTE_OP(CEE_LDLOC_0);
        EXECUTE_OP(CEE_LDLOC_1);
        EXECUTE_OP(CEE_LDLOC_2);
        EXECUTE_OP(CEE_LDLOC_3);
        EXECUTE_OP(CEE_STLOC);
        EXECUTE_OP(CEE_STLOC_S);
        EXECUTE_OP(CEE_STLOC_0);
        EXECUTE_OP(CEE_STLOC_1);
        EXECUTE_OP(CEE_STLOC_2);
        EXECUTE_OP(CEE_STLOC_3);
        EXECUTE_OP(CEE_LDC_I4);
        EXECUTE_OP(CEE_LDC_I4_S);
        EXECUTE_OP(CEE_LDC_I4_0);
        EXECUTE_OP(CEE_LDC_I4_1);
        EXECUTE_OP(CEE_LDC_I4_2);
        EXECUTE_OP(CEE_LDC_I4_3);
        EXECUTE_OP(CEE_LDC_I4_4);
        EXECUTE_OP(CEE_LDC_I4_5);
        EXECUTE_OP(CEE_LDC_I4_6);
        EXECUTE_OP(CEE_LDC_I4_7);
        EXECUTE_OP(CEE_LDC_I4_8);
        EXECUTE_OP(CEE_LDC_R4);
        EXECUTE_OP(CEE_LDC_R8);
        EXECUTE_OP(CEE_CONV_R4);
        EXECUTE_OP(CEE_BR_S);
        EXECUTE_OP(CEE_NEWOBJ);
        EXECUTE_OP(CEE_NEWARR);
        EXECUTE_OP(CEE_DUP);
        EXECUTE_OP(CEE_LDFLD);
        EXECUTE_OP(CEE_STFLD);
        EXECUTE_OP(CEE_STELEM_I1);
        EXECUTE_OP(CEE_STELEM_I2);
        EXECUTE_OP(CEE_STELEM_I4);
        EXECUTE_OP(CEE_STELEM_I8);
        EXECUTE_OP(CEE_STELEM_R4);
        EXECUTE_OP(CEE_STELEM_R8);
        EXECUTE_OP(CEE_STELEM_I);
        EXECUTE_OP(CEE_STELEM_REF);
    default:
        assert(!"Invalid OpCode");
        break;
    }
}

void Interpreter::ExecuteOp_LDARG(size_t index)
{
    TypeDesc srcType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = calleeInfo.GetArg(index, srcType);
    evalStack_.PushVar(src, srcType);
}

void Interpreter::ExecuteOp_LDLOC(size_t index)
{
    TypeDesc srcType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = calleeInfo.GetLocalVar(index, srcType);
    evalStack_.PushVar(src, srcType);
}

void Interpreter::ExecuteOp_STLOC(size_t index)
{
    TypeDesc srcType, destType;
    auto& calleeInfo = calleeInfo_.top();
    auto src = evalStack_.GetTopVar(srcType);
    auto dest = calleeInfo.GetLocalVar(0, destType);
    assert(srcType.GetStackSize() == destType.GetStackSize());
    memcpy(dest, src, srcType.GetStackSize());
    evalStack_.PopVar();
}

void Interpreter::ExecuteOp_LDC_I4(int32_t i)
{
    evalStack_.PushVar(&i, { tda_Normal, ELEMENT_TYPE_I4, 0, CorlibBinder::Current().Types.Int32 });
}

void Interpreter::ExecuteOp_STELEM(CorElementType type)
{
    TypeDesc valType, arrType;
    auto valSrc = evalStack_.PopTopVar(valType);

    auto index = PopIndexOrArrayLength();
    auto arr = *reinterpret_cast<ObjectRef<Array>*>(evalStack_.PopTopVar(arrType));
    auto elemType = ClassToType(arr->GetHeader().Class);
    assert(elemType.Flags | tda_SzArray);
    assert(type != ELEMENT_TYPE_BYREF || elemType.Type == ELEMENT_TYPE_BYREF);

    union {
        uint64_t ival;
        double rval;
        uintptr_t refval;
    } val;
    bool isRealVal = false;

    switch (type)
    {
    case ELEMENT_TYPE_I1:
        val.ival = *reinterpret_cast<uint8_t*>(valSrc);
        break;
    case ELEMENT_TYPE_I2:
        val.ival = *reinterpret_cast<uint16_t*>(valSrc);
        break;
    case ELEMENT_TYPE_I4:
        val.ival = *reinterpret_cast<uint32_t*>(valSrc);
        break;
    case ELEMENT_TYPE_I8:
        val.ival = *reinterpret_cast<uint64_t*>(valSrc);
        break;
    case ELEMENT_TYPE_R4:
        val.rval = *reinterpret_cast<float*>(valSrc);
        isRealVal = true;
        break;
    case ELEMENT_TYPE_R8:
        val.rval = *reinterpret_cast<double*>(valSrc);
        isRealVal = true;
        break;
    case ELEMENT_TYPE_BYREF:
        val.refval = *reinterpret_cast<uintptr_t*>(valSrc);
        break;
    default:
        assert(!"Invalid type.");
        break;
    }

    if (!isRealVal && (arrType.Type == ELEMENT_TYPE_R4 || arrType.Type == ELEMENT_TYPE_R8))
        val.rval = static_cast<double>(val.ival);
    if (isRealVal && (arrType.Type >= ELEMENT_TYPE_BOOLEAN && arrType.Type <= ELEMENT_TYPE_U8))
        val.ival = (uint64_t)round(val.rval);

    switch (elemType.Type)
    {
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
        arr->At<uint8_t>(index) = static_cast<uint8_t>(val.ival);
        break;
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
        arr->At<uint16_t>(index) = static_cast<uint16_t>(val.ival);
        break;
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
        arr->At<uint32_t>(index) = static_cast<uint32_t>(val.ival);
        break;
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        arr->At<uint64_t>(index) = val.ival;
        break;
    case ELEMENT_TYPE_R4:
        arr->At<float>(index) = static_cast<float>(val.rval);
        break;
    case ELEMENT_TYPE_R8:
        arr->At<double>(index) = val.rval;
        break;
    case ELEMENT_TYPE_BYREF:
        arr->At<uintptr_t>(index) = val.refval;
        break;
    default:
        assert(!"Invalid type.");
        break;
    }
}

uint64_t Interpreter::PopIndexOrArrayLength()
{
    TypeDesc lenType;
    auto lenSrc = evalStack_.GetTopVar(lenType);
    uint64_t len;
    switch (lenType.Type)
    {
    case ELEMENT_TYPE_I4:
        len = *reinterpret_cast<int32_t*>(lenSrc);
        break;
    case ELEMENT_TYPE_I:
        len = *reinterpret_cast<intptr_t*>(lenSrc);
        break;
    default:
        assert(!"Invalid num elements type");
        break;
    }
    evalStack_.PopVar();
    return len;
}
