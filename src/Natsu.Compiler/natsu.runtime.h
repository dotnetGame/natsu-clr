// natsu clr runtime
#pragma once
#include <cstdint>

namespace natsu
{
    inline constexpr float to_float(uint32_t value) noexcept
    {
        return static_cast<const float&>(value);
    }
    
    inline constexpr double to_double(uint64_t value) noexcept
    {
        return static_cast<const double&>(value);
    }
    
    inline constexpr int64_t to_int64(uint64_t value) noexcept
    {
        return static_cast<const int64_t&>(value);
    }

    template<class T>
    struct gc_ptr
    {
        T *ptr_;
    };

    template<class T>
    struct sz_array
    {

    };
}
