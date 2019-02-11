//
// Natsu CLR VM
//
#pragma once
#include "../loader/AssemblyLoader.hpp"
#include "../md/mddefs.hpp"
#include "EvaluationStack.hpp"
#include "opinfo.h"

namespace clr
{
namespace vm
{
    class ExecutionException : std::exception
    {
        using exception::exception;
    };

    class Interpreter
    {
    public:
        Interpreter(loader::AssemblyLoader& assemblyLoader);

        void ExecuteMethod(const MethodDesc& method);

    private:
        void ExecuteOp(OpInfo& op, OpArgsVal& args);
        void ExecuteILMethod(const MethodDesc& method);

        template <opcode_t OpCode>
        void ExecuteOp(OpInfo& op, OpArgsVal& args);

    private:
        void ExecuteOp_LDARG(size_t index);
        void ExecuteOp_LDLOC(size_t index);
        void ExecuteOp_STLOC(size_t index);
        void ExecuteOp_LDC_I4(int32_t i);
        void ExecuteOp_STELEM(clr::metadata::CorElementType type);
        uint64_t PopIndexOrArrayLength();
    private:
        EvaluationStack evalStack_;
        loader::AssemblyLoader& assemblyLoader_;

        std::stack<const uint8_t*> IP_;
        std::stack<CalleeInfo> calleeInfo_;
    };
}
}
