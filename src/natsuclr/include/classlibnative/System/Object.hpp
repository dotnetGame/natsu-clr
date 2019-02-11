//
// Natsu CLR Class Lib Native
//
#pragma once
#include "../../vm/EEClass.hpp"

namespace clr
{
namespace corlib
{
    class Object;

    struct ObjectHeader
    {
        vm::EEClass* Class;
    };

    template <class T = Object>
    class ObjectRef
    {
    public:
        explicit ObjectRef(T* ptr)
            : ptr_(ptr)
        {
        }

        T* Get() const noexcept
        {
            return ptr_;
        }

        T* operator->() const noexcept
        {
            return Get();
        }

    private:
        T* ptr_;
    };

    class Object
    {
    public:
        ObjectHeader& GetHeader() noexcept;

        static uintptr_t GetType(ObjectRef<> this_);
    };
}
}
