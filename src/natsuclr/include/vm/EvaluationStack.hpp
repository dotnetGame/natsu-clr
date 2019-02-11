//
// Natsu CLR VM
//
#pragma once
#include "EEClass.hpp"
#include <algorithm>
#include <stack>
#include <type_traits>
#include <vector>

namespace clr
{
namespace vm
{
    class StackOverflowException : std::exception
    {
        using exception::exception;
    };

    class EvaluationStack
    {
        struct FrameMarker
        {
            const MethodDesc& Method;
            size_t VarsOffset;
        };
    public:
        EvaluationStack(size_t stackSize = 4096);

        void PushVar(const void* value, const TypeDesc& type);
        uint8_t* GetTopVar(TypeDesc& type);
        void PopVar();
        uint8_t* PopTopVar(TypeDesc& type);

        void PushFrame(const MethodDesc& method);
        void PopFrame();

    private:
        std::unique_ptr<uint8_t[]> stack_;
        size_t stackPointer_;
        size_t stackSize_;
        std::vector<VarDesc> stackVars_;
        std::stack<FrameMarker> frames_;
    };

    class CalleeInfo
    {
    public:
        CalleeInfo(const MethodDesc& method);

        uint8_t* GetArg(size_t index, TypeDesc& type);
        uint8_t* GetLocalVar(size_t index, TypeDesc& type);
        uint8_t* GetRet(TypeDesc& type);

    private:
        const MethodDesc& method_;
        std::unique_ptr<uint8_t[]> data_;
        size_t argPointer_;
        size_t localVarPointer_;
        size_t retPointer_;
    };
}
}
