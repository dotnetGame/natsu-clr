// natsu clr runtime
#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>
#include <cassert>

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

    inline constexpr void nop() noexcept
    {
    }

    template<class T>
    struct gc_ptr
    {
        T* ptr_;

        gc_ptr()
            : ptr_(nullptr)
        {
        }

        template<class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
        gc_ptr(gc_ptr<U>&& other)
            : ptr_(static_cast<T*>(other.ptr_))
        {
            other.ptr_ = nullptr;
        }

        template<class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
        gc_ptr(const gc_ptr<U>& other)
            : ptr_(static_cast<T*>(other.ptr_))
        {
        }

        T* operator->() const
        {
            assert(ptr_);
            return ptr_;
        }
    };

    template<class T>
    struct sz_array
    {

    };

    template<class T>
    gc_ptr<T> gc_new()
    {
        return {};
    }

    template<class T, bool IsValueType, class ...TArgs>
    auto make_object(TArgs... args)
    {
        if constexpr (IsValueType)
        {
            T value;
            value._ctor(value, std::move(args)...);
            return value;
        }
        else
        {
            auto value = gc_new<T>();
            value->_ctor(value, std::move(args)...);
            return value;
        }
    }
}
