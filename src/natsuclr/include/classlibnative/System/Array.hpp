//
// Natsu CLR Class Lib Native
//
#pragma once
#include "Object.hpp"

namespace clr
{
namespace corlib
{
    class Array : public Object
    {
    public:
        static long get_LongLength(ObjectRef<Array> this_);
        static int GetLength(ObjectRef<Array> this_, int dimension);
        static int get_Rank(ObjectRef<Array> this_);
        static int get_Length(ObjectRef<Array> this_);

        template <typename T>
        T& At(uint64_t index) noexcept
        {
            auto cls = GetHeader().Class;
            auto offset = uintptr_t(this) + sizeof(Array) + align(cls->InstanceSize, cls->Align) * index;
            return *reinterpret_cast<T*>(offset);
        }

        uint64_t length_;
    };
}
}
