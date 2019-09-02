// natsu clr runtime
#pragma once
#include "natsu.typedef.h"

using namespace std::string_view_literals;

namespace natsu
{
namespace stack
{
    struct int32
    {
        int32_t value_;

        constexpr int32(int32_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }
    };

    struct int64
    {
        int64_t value_;

        constexpr int64(int64_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }
    };

    struct native_int
    {
        intptr_t value_;

        constexpr native_int(intptr_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }
    };

    struct F
    {
        double value_;

        constexpr F(double value)
            : value_(value) {}
    };

    struct Ref
    {
        uintptr_t value_;

        constexpr Ref(uintptr_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }
    };

    struct O
    {
        intptr_t value_;

        constexpr O(uintptr_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }
    };

    static constexpr O null = 0;

    namespace details
    {
        template <class T>
        struct stack_from_impl
        {
            T operator()(T &&value)
            {
                return value;
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CorLib::System::Boolean>
        {
            int32 operator()(const ::System_Private_CorLib::System::Boolean &value) const noexcept
            {
                return { value.m_value };
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CorLib::System::Int32>
        {
            int32 operator()(const ::System_Private_CorLib::System::Int32 &value) const noexcept
            {
                return { value.m_value };
            }
        };

        template <class T>
        struct stack_from_impl<gc_obj_ref<T>>
        {
            O operator()(const gc_obj_ref<T> &value) const noexcept
            {
                return { static_cast<uintptr_t>(value) };
            }
        };

        template <class TFrom, class TTo>
        struct stack_to_impl
        {
            gc_obj_ref<TTo> operator()(const O &value) const noexcept
            {
                return gc_obj_ref<TTo>(reinterpret_cast<TTo *>(value.value_));
            }

            TTo operator()(const int32 &value) const noexcept
            {
                return TTo { value.value_ };
            }
        };

        template <class T>
        struct box_impl
        {
            O operator()(T &&value)
            {
                auto box = gc_new<::System_Private_CorLib::System::Box_1<T>>();
                box->value__ = value;
                return stack_from(box);
            }
        };

        template <class T>
        struct box_impl<::System_Private_CorLib::System::Nullable_1<T>>
        {
            O operator()(const ::System_Private_CorLib::System::Nullable_1<T> &value) const noexcept
            {
                if (value.hasValue)
                {
                    auto box = gc_new<::System_Private_CorLib::System::Box_1<T>>();
                    box->value__ = value;
                    return stack_from(box);
                }
                else
                {
                    return null;
                }
            }
        };
    }
}

template <class TFrom>
auto stack_from(TFrom &&value)
{
    return stack::details::stack_from_impl<std::decay_t<TFrom>>()(std::forward<TFrom>(value));
}

template <class TTo, class TFrom>
auto stack_to(TFrom &&value)
{
    return stack::details::stack_to_impl<std::decay_t<TFrom>, TTo>()(std::forward<TFrom>(value));
}

void check_null_obj_ref(stack::O obj)
{
    if (!obj)
        throw_null_ref_exception();
}

gc_obj_ref<object> gc_alloc(const vtable_t &vtable, size_t size);

template <class T>
gc_obj_ref<T> gc_new(size_t size)
{
    auto obj = gc_alloc(static_holder<typename T::VTable>::value, size);
    return obj.template cast<T>();
}

template <class T>
gc_obj_ref<T> gc_new()
{
    return gc_new<T>(sizeof(T));
}

template <class T>
gc_obj_ref<::System_Private_CorLib::System::SZArray_1<T>> gc_new_array(stack::int32 length)
{
    using obj_t = ::System_Private_CorLib::System::SZArray_1<T>;
    auto size = sizeof(obj_t) + (size_t)length * sizeof(T);
    auto obj = gc_new<obj_t>(size);
    obj->Length = length;
    return obj;
}

gc_obj_ref<::System_Private_CorLib::System::String> load_string(std::u16string_view string);
std::u16string_view to_string_view(gc_obj_ref<::System_Private_CorLib::System::String> string);

template <class T, class... TArgs>
auto make_object(TArgs... args)
{
    if constexpr (is_value_type_v<T>)
    {
        T value;
        value._ctor(value, std::forward<TArgs>(args)...);
        return value;
    }
    else
    {
        auto value = gc_new<T>();
        value->_ctor(value, std::forward<TArgs>(args)...);
        return value;
    }
}

template <class T>
natsu_exception make_exception(gc_obj_ref<T> exception)
{
    return { std::move(exception) };
}

template <class T, class... TArgs>
[[noreturn]] void throw_exception(TArgs &&... args)
{
    throw make_exception(make_object<T>(std::forward<TArgs>(args)...));
}

namespace ops
{
    inline stack::int32 add(const stack::int32 &lhs, const stack::int32 &rhs) noexcept
    {
        return lhs.value_ + rhs.value_;
    }

    inline stack::int32 sub(const stack::int32 &lhs, const stack::int32 &rhs) noexcept
    {
        return lhs.value_ - rhs.value_;
    }

    inline stack::int32 clt(const stack::int32 &lhs, const stack::int32 &rhs) noexcept
    {
        return lhs.value_ < rhs.value_ ? 1 : 0;
    }

    inline stack::int32 ceq(const stack::int32 &lhs, const stack::int32 &rhs) noexcept
    {
        return lhs.value_ == rhs.value_ ? 1 : 0;
    }

    inline stack::int32 cgt_un(const stack::int32 &lhs, const stack::int32 &rhs) noexcept
    {
        return static_cast<uint32_t>(lhs.value_) > static_cast<uint32_t>(rhs.value_) ? 1 : 0;
    }

    stack::native_int ldlen(const stack::O &obj);

    inline stack::int32 conv_i4(stack::int32 value) noexcept
    {
        return value;
    }

    inline stack::int32 conv_i4(stack::int64 value) noexcept
    {
        return static_cast<int32_t>(value.value_);
    }

    inline stack::int32 conv_i4(stack::F value) noexcept
    {
        return static_cast<int32_t>(value.value_);
    }

    template <class T, class... TArgs>
    auto newobj(TArgs... args)
    {
        if constexpr (is_value_type_v<T>)
        {
            T value;
            value._ctor(value, std::forward<TArgs>(args)...);
            return stack_from(value);
        }
        else
        {
            auto value = gc_new<T>();
            value->_ctor(value, std::forward<TArgs>(args)...);
            return stack_from(value);
        }
    }

    template <class TFrom>
    stack::O box(TFrom &&value) noexcept
    {
        return stack::details::box_impl()(std::forward<TFrom>(value));
    }

    template <class TFrom>
    stack::Ref ref(TFrom &value) noexcept
    {
        return reinterpret_cast<uintptr_t>(&value);
    }

    template <class TFrom>
    void initobj(const stack::Ref &addr) noexcept
    {
        std::memset((void *)addr.value_, 0, sizeof(TFrom));
    }

    template <class TFrom>
    void initobj(const stack::native_int &addr) noexcept
    {
        std::memset((void *)addr.value_, 0, sizeof(TFrom));
    }

    [[noreturn]] void throw_(const stack::O &obj);
}

namespace details
{
    template <class T, bool IsValueType>
    struct box_impl;

    template <class T>
    struct box_impl<T, true>
    {
        gc_obj_ref<::System_Private_CorLib::System::Box_1<T>> operator()(const T &value)
        {
            auto box = gc_new<::System_Private_CorLib::System::Box_1<T>>();
            box->value__ = value;
            return box;
        }
    };

    template <class T, bool IsValueType>
    struct unbox_any_impl;

    template <class T>
    struct unbox_any_impl<T, true>
    {
        T operator()(gc_obj_ref<object> value)
        {
            auto box = value.as<::System_Private_CorLib::System::Box_1<T>>();
            return box->value__;
        }
    };
}

template <class T>
auto box(T &value)
{
    return details::box_impl<T, is_value_type_v<T>>()(value);
}

template <class T, class TArg>
auto unbox_any(TArg &value)
{
    return details::unbox_any_impl<T, is_value_type_v<T>>()(value);
}

template <class T>
gc_ref<T> unbox(gc_obj_ref<object> value)
{
    auto box = value.as<::System_Private_CorLib::System::Box_1<T>>();
    check_null_obj_ref(box);
    return gc_ref_from_ref(box->value__);
}

template <class T>
gc_ref<T> unbox_exact(gc_obj_ref<object> value)
{
    auto box = value.cast<::System_Private_CorLib::System::Box_1<T>>();
    return gc_ref_from_ref(box->value__);
}

template <class T>
void init_obj(T &value)
{
    std::memset(&value, 0, sizeof(value));
}
}
