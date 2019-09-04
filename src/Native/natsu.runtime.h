// natsu clr runtime
#pragma once
#include "natsu.typedef.h"
#include <cmath>
#include <limits>

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

        constexpr bool istrue() const noexcept
        {
            return value_;
        }
    };

    struct int64
    {
        int64_t value_;

        constexpr int64(int64_t value)
            : value_(value) {}

        constexpr bool istrue() const noexcept
        {
            return value_;
        }
    };

    struct native_int
    {
        intptr_t value_;

        constexpr native_int(intptr_t value)
            : value_(value) {}

        constexpr bool istrue() const noexcept
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

        constexpr bool istrue() const noexcept
        {
            return value_;
        }
    };

    struct O
    {
        intptr_t value_;

        constexpr O(uintptr_t value)
            : value_(value) {}

        constexpr bool istrue() const noexcept
        {
            return value_;
        }

        object_header &header() const noexcept
        {
            return *reinterpret_cast<object_header *>(reinterpret_cast<uint8_t *>(value_) - sizeof(object_header));
        }
    };

    static constexpr O null = 0;

    namespace details
    {
        template <class T>
        struct stack_from_impl;

        template <class T>
        struct stack_from_impl<gc_obj_ref<T>>
        {
            O operator()(const gc_obj_ref<T> &value) const noexcept
            {
                return { static_cast<uintptr_t>(value) };
            }
        };

        template <class T>
        struct stack_from_impl<gc_ref<T>>
        {
            Ref operator()(const gc_ref<T> &value) const noexcept
            {
                return { static_cast<uintptr_t>(value) };
            }
        };

        template <class T>
        struct stack_from_impl<gc_ptr<T>>
        {
            native_int operator()(const gc_ptr<T> &value) const noexcept
            {
                return static_cast<intptr_t>(static_cast<uintptr_t>(value));
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CorLib::System::IntPtr>
        {
            native_int operator()(const ::System_Private_CorLib::System::IntPtr &value) const noexcept
            {
                return (intptr_t) static_cast<uintptr_t>(value._value);
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CorLib::System::UIntPtr>
        {
            native_int operator()(const ::System_Private_CorLib::System::UIntPtr &value) const noexcept
            {
                return (intptr_t) static_cast<uintptr_t>(value._value);
            }
        };

#define DEFINE_STACK_FROM_CAST(From, To, Med, Cast)                    \
    template <>                                                        \
    struct stack_from_impl<From>                                       \
    {                                                                  \
        To operator()(const From &value) const noexcept                \
        {                                                              \
            return static_cast<Cast>(static_cast<Med>(value.m_value)); \
        }                                                              \
    };

        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Boolean, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::SByte, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Byte, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Char, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Int16, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::UInt16, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Int32, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::UInt32, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Int64, int64, int64_t, int64_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::UInt64, int64, uint64_t, int64_t);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Single, F, float, double);
        DEFINE_STACK_FROM_CAST(::System_Private_CorLib::System::Double, F, double, double);

#undef DEFINE_STACK_FROM_CAST

        template <class T>
        struct stack_from_impl
        {
            auto operator()(T value)
            {
                if constexpr (natsu::is_enum_v<T>)
                    return stack_from_impl<decltype(value.value__)>()(value.value__);
                else
                    return value;
            }
        };

        template <class TFrom, class TTo>
        struct stack_to_impl
        {
            TTo operator()(TFrom value)
            {
                if constexpr (natsu::is_enum_v<TTo>)
                    return static_cast<TTo>(value.value_);
                else
                    return value;
            }
        };

        template <class TTo>
        struct stack_to_impl<O, gc_obj_ref<TTo>>
        {
            gc_obj_ref<TTo> operator()(const O &value) const noexcept
            {
                return gc_obj_ref<TTo>(reinterpret_cast<TTo *>(value.value_));
            }
        };

        template <class TTo>
        struct stack_to_impl<Ref, gc_ref<TTo>>
        {
            gc_ref<TTo> operator()(const Ref &value) const noexcept
            {
                return gc_ref<TTo>(*reinterpret_cast<TTo *>(value.value_));
            }
        };

        template <class TTo>
        struct stack_to_impl<Ref, gc_ptr<TTo>>
        {
            gc_ptr<TTo> operator()(const Ref &value) const noexcept
            {
                return gc_ptr<TTo>(reinterpret_cast<TTo *>(value.value_));
            }
        };

        template <class TTo>
        struct stack_to_impl<native_int, gc_ref<TTo>>
        {
            gc_ref<TTo> operator()(const native_int &value) const noexcept
            {
                return gc_ref<TTo>(*reinterpret_cast<TTo *>(value.value_));
            }
        };

        template <class TTo>
        struct stack_to_impl<native_int, gc_ptr<TTo>>
        {
            gc_ptr<TTo> operator()(const native_int &value) const noexcept
            {
                return gc_ptr<TTo>(reinterpret_cast<TTo *>(value.value_));
            }
        };

#define DEFINE_STACK_TO_CAST(From, To, Med)             \
    template <>                                         \
    struct stack_to_impl<From, To>                      \
    {                                                   \
        To operator()(const From &value) const noexcept \
        {                                               \
            return static_cast<Med>(value.value_);      \
        }                                               \
    };
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::Boolean, bool);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::SByte, int8_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::Byte, uint8_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::Char, char16_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::Int16, int16_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::UInt16, uint16_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::Int32, int32_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::UInt32, uint32_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::IntPtr, intptr_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CorLib::System::UIntPtr, uintptr_t);

        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::Boolean, bool);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::SByte, int8_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::Byte, uint8_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::Char, char16_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::Int16, int16_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::UInt16, uint16_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::Int32, int32_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::UInt32, uint32_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::IntPtr, intptr_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CorLib::System::UIntPtr, uintptr_t);

        DEFINE_STACK_TO_CAST(int64, ::System_Private_CorLib::System::Int64, int64_t);
        DEFINE_STACK_TO_CAST(int64, ::System_Private_CorLib::System::UInt64, uint64_t);

        DEFINE_STACK_TO_CAST(F, ::System_Private_CorLib::System::Single, float);
        DEFINE_STACK_TO_CAST(F, ::System_Private_CorLib::System::Double, double);

#undef DEFINE_STACK_TO_CAST

        template <class T>
        struct box_impl
        {
            O operator()(T value)
            {
                auto box = gc_new<T>();
                *box = value;
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
                    auto box = gc_new<T>();
                    box->value__ = value;
                    return stack_from(box);
                }
                else
                {
                    return null;
                }
            }
        };

        template <class T>
        struct unbox_impl
        {
            Ref operator()(const O &obj)
            {
                using ::System_Private_CorLib::System::Object;

                check_null_obj_ref(obj);
                auto box = stack_to<gc_obj_ref<Object>>(obj).as<T>();
                if (box)
                    return stack_from(gc_ref_from_ref(*box));
                else
                    throw_invalid_cast_exception();
            }
        };

        template <class T>
        struct unbox_impl<::System_Private_CorLib::System::Nullable_1<T>>
        {
            Ref operator()(const O &obj)
            {
                using ::System_Private_CorLib::System::Nullable_1;
                using ::System_Private_CorLib::System::Object;

                if (obj)
                {
                    auto box = stack_to<gc_obj_ref<Object>>(obj).as<T>();
                    if (box)
                    {
                        auto new_obj = gc_new<Nullable_1<T>>();
                        new_obj->value = *box;
                        return stack_from(gc_ref_from_ref(*new_obj));
                    }
                    else
                    {
                        throw_invalid_cast_exception();
                    }
                }
                else
                {
                    auto new_obj = gc_new<Nullable_1<T>>();
                    return stack_from(gc_ref_from_ref(*new_obj));
                }
            }
        };

        template <class T>
        struct unbox_any_impl
        {
            auto operator()(const O &obj)
            {
                using ::System_Private_CorLib::System::Object;

                check_null_obj_ref(obj);
                auto box = stack_to<gc_obj_ref<Object>>(obj).as<T>();
                if (box)
                {
                    if constexpr (natsu::is_value_type_v<T>)
                    {
                        return *box;
                    }
                    else
                    {
                        return obj;
                    }
                }
                else
                {
                    throw_invalid_cast_exception();
                }
            }
        };

        template <class T>
        struct unbox_any_impl<::System_Private_CorLib::System::Nullable_1<T>>
        {
            ::System_Private_CorLib::System::Nullable_1<T> operator()(const O &obj)
            {
                using ::System_Private_CorLib::System::Nullable_1;
                using ::System_Private_CorLib::System::Object;

                if (obj)
                {
                    auto box = stack_to<gc_obj_ref<Object>>(obj).as<T>();
                    if (box)
                    {
                        return make_object<Nullable_1<T>>(*box);
                    }
                    else
                    {
                        throw_invalid_cast_exception();
                    }
                }
                else
                {
                    return make_object<Nullable_1<T>>();
                }
            }
        };

        template <class T>
        struct isinst_impl
        {
            O operator()(const O &obj) const noexcept
            {
                if (obj.istrue() && obj.header().vtable_as<typename T::VTable>())
                    return obj;
                return null;
            }
        };

        template <class T>
        struct isinst_impl<::System_Private_CorLib::System::Nullable_1<T>>
        {
            O operator()(const O &obj) const noexcept
            {
                return isinst_impl<T>()(obj);
            }
        };

        template <class TFrom, class TTo>
        struct access_impl;

        template <class TFrom, class TTo>
        struct access_impl<TFrom, gc_ref<TTo>>
        {
            gc_ref<TTo> operator()(const TFrom &obj) const noexcept
            {
                return gc_ref_from_ref(stack_to<TTo>(obj));
            }
        };

        template <class TTo>
        struct access_impl<Ref, gc_ref<TTo>>
        {
            gc_ref<TTo> operator()(const Ref &obj) const noexcept
            {
                return stack_to<gc_ref<TTo>>(obj);
            }
        };

        template <class TTo>
        struct access_impl<O, TTo>
        {
            TTo operator()(const O &obj) const noexcept
            {
                return stack_to<TTo>(obj);
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

inline void check_null_obj_ref(stack::O obj)
{
    if (!obj.istrue())
        throw_null_ref_exception();
}

inline void check_null_obj_ref(stack::native_int addr)
{
    if (!addr.istrue())
        throw_null_ref_exception();
}

inline void check_null_obj_ref(stack::Ref addr)
{
    if (!addr.istrue())
        throw_null_ref_exception();
}

gc_obj_ref<::System_Private_CorLib::System::Object> gc_alloc(const vtable_t &vtable, size_t size);

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
    auto size = sizeof(obj_t) + (size_t)length.value_ * sizeof(T);
    auto obj = gc_new<obj_t>(size);
    obj->Length = length.value_;
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
#define BINARY_OP_IMPL(name, op, A, B, Ret, Med, Cast)                                          \
    inline Ret name(const A &lhs, const B &rhs) noexcept                                        \
    {                                                                                           \
        return static_cast<Cast>(static_cast<Med>(lhs.value_) op static_cast<Med>(rhs.value_)); \
    }

    BINARY_OP_IMPL(add, +, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(add, +, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(add, +, stack::int32, stack::Ref, stack::Ref, intptr_t, uintptr_t);
    BINARY_OP_IMPL(add, +, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(add, +, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(add, +, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(add, +, stack::native_int, stack::Ref, stack::Ref, intptr_t, uintptr_t);
    BINARY_OP_IMPL(add, +, stack::F, stack::F, stack::F, double, double);
    BINARY_OP_IMPL(add, +, stack::Ref, stack::int32, stack::Ref, intptr_t, uintptr_t);
    BINARY_OP_IMPL(add, +, stack::Ref, stack::native_int, stack::Ref, intptr_t, uintptr_t);

    BINARY_OP_IMPL(sub, -, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(sub, -, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(sub, -, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(sub, -, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(sub, -, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(sub, -, stack::F, stack::F, stack::F, double, double);
    BINARY_OP_IMPL(sub, -, stack::Ref, stack::int32, stack::Ref, intptr_t, uintptr_t);
    BINARY_OP_IMPL(sub, -, stack::Ref, stack::native_int, stack::Ref, intptr_t, uintptr_t);
    BINARY_OP_IMPL(sub, -, stack::Ref, stack::Ref, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(mul, *, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(mul, *, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(mul, *, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(mul, *, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(mul, *, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(mul, *, stack::F, stack::F, stack::F, double, double);

    BINARY_OP_IMPL(div, /, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(div, /, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(div, /, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(div, /, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(div, /, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(div, /, stack::F, stack::F, stack::F, double, double);

    BINARY_OP_IMPL(rem, %, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(rem, %, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(rem, %, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(rem, %, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(rem, %, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(and, &, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(and, &, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(and, &, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(and, &, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(and, &, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(or, |, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(or, |, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(or, |, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(or, |, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(or, |, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(xor, ^, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(xor, ^, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(xor, ^, stack::int64, stack::int64, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(xor, ^, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(xor, ^, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(shl, <<, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(shl, <<, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(shl, <<, stack::int64, stack::int32, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(shl, <<, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(shl, <<, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(shr, >>, stack::int32, stack::int32, stack::int32, int32_t, int32_t);
    BINARY_OP_IMPL(shr, >>, stack::int32, stack::native_int, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(shr, >>, stack::int64, stack::int32, stack::int64, int64_t, int64_t);
    BINARY_OP_IMPL(shr, >>, stack::native_int, stack::int32, stack::native_int, intptr_t, intptr_t);
    BINARY_OP_IMPL(shr, >>, stack::native_int, stack::native_int, stack::native_int, intptr_t, intptr_t);

    BINARY_OP_IMPL(div_un, /, stack::int32, stack::int32, stack::int32, uint32_t, int32_t);
    BINARY_OP_IMPL(div_un, /, stack::int32, stack::native_int, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(div_un, /, stack::int64, stack::int64, stack::int64, uint64_t, int64_t);
    BINARY_OP_IMPL(div_un, /, stack::native_int, stack::int32, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(div_un, /, stack::native_int, stack::native_int, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(div_un, /, stack::F, stack::F, stack::F, double, double);

    BINARY_OP_IMPL(rem_un, %, stack::int32, stack::int32, stack::int32, uint32_t, int32_t);
    BINARY_OP_IMPL(rem_un, %, stack::int32, stack::native_int, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(rem_un, %, stack::int64, stack::int64, stack::int64, uint64_t, int64_t);
    BINARY_OP_IMPL(rem_un, %, stack::native_int, stack::int32, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(rem_un, %, stack::native_int, stack::native_int, stack::native_int, uintptr_t, intptr_t);

    BINARY_OP_IMPL(shr_un, >>, stack::int32, stack::int32, stack::int32, uint32_t, int32_t);
    BINARY_OP_IMPL(shr_un, >>, stack::int32, stack::native_int, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(shr_un, >>, stack::int64, stack::int32, stack::int64, uint64_t, int64_t);
    BINARY_OP_IMPL(shr_un, >>, stack::native_int, stack::int32, stack::native_int, uintptr_t, intptr_t);
    BINARY_OP_IMPL(shr_un, >>, stack::native_int, stack::native_int, stack::native_int, uintptr_t, intptr_t);

#undef BINARY_OP_IMPL

    inline stack::F rem(const stack::F &lhs, const stack::F &rhs) noexcept
    {
        return fmod(lhs.value_, rhs.value_);
    }

#define UNARY_OP_IMPL(name, op, A, Med, Cast)                        \
    inline A name(const A &value) noexcept                           \
    {                                                                \
        return static_cast<Cast>(op static_cast<Med>(value.value_)); \
    }

    UNARY_OP_IMPL(neg, -, stack::int32, int32_t, int32_t);
    UNARY_OP_IMPL(neg, -, stack::int64, int64_t, int64_t);
    UNARY_OP_IMPL(neg, -, stack::native_int, intptr_t, intptr_t);
    UNARY_OP_IMPL(neg, -, stack::F, double, double);

#undef UNARY_OP_IMPL

#define COMPARE_OP_IMPL(name, op, A, B, Med)                                         \
    inline stack::int32 name(const A &lhs, const B &rhs) noexcept                    \
    {                                                                                \
        return static_cast<Med>(lhs.value_) op static_cast<Med>(rhs.value_) ? 1 : 0; \
    }

    COMPARE_OP_IMPL(clt, <, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(clt, <, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(clt, <, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(clt, <, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(clt, <, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(clt, <, stack::F, stack::F, double);
    COMPARE_OP_IMPL(clt, <, stack::Ref, stack::Ref, intptr_t);

    COMPARE_OP_IMPL(cle, <=, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(cle, <=, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cle, <=, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(cle, <=, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(cle, <=, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cle, <=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cle, <=, stack::Ref, stack::Ref, intptr_t);

    COMPARE_OP_IMPL(ceq, ==, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(ceq, ==, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(ceq, ==, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::native_int, stack::Ref, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::F, stack::F, double);
    COMPARE_OP_IMPL(ceq, ==, stack::Ref, stack::Ref, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::Ref, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(ceq, ==, stack::O, stack::O, intptr_t);

    COMPARE_OP_IMPL(cge, >=, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(cge, >=, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cge, >=, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(cge, >=, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(cge, >=, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cge, >=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cge, >=, stack::Ref, stack::Ref, intptr_t);

    COMPARE_OP_IMPL(cgt, >, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(cgt, >, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cgt, >, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(cgt, >, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(cgt, >, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cgt, >, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cgt, >, stack::Ref, stack::Ref, intptr_t);

    COMPARE_OP_IMPL(cne, !=, stack::int32, stack::int32, int32_t);
    COMPARE_OP_IMPL(cne, !=, stack::int32, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cne, !=, stack::int64, stack::int64, int64_t);
    COMPARE_OP_IMPL(cne, !=, stack::native_int, stack::int32, intptr_t);
    COMPARE_OP_IMPL(cne, !=, stack::native_int, stack::native_int, intptr_t);
    COMPARE_OP_IMPL(cne, !=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cne, !=, stack::Ref, stack::Ref, intptr_t);
    COMPARE_OP_IMPL(cne, !=, stack::O, stack::O, intptr_t);

    COMPARE_OP_IMPL(clt_un, <, stack::int32, stack::int32, uint32_t);
    COMPARE_OP_IMPL(clt_un, <, stack::int32, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(clt_un, <, stack::int64, stack::int64, uint64_t);
    COMPARE_OP_IMPL(clt_un, <, stack::native_int, stack::int32, uintptr_t);
    COMPARE_OP_IMPL(clt_un, <, stack::native_int, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(clt_un, <, stack::F, stack::F, double);
    COMPARE_OP_IMPL(clt_un, <, stack::Ref, stack::Ref, uintptr_t);

    COMPARE_OP_IMPL(cle_un, <=, stack::int32, stack::int32, uint32_t);
    COMPARE_OP_IMPL(cle_un, <=, stack::int32, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cle_un, <=, stack::int64, stack::int64, uint64_t);
    COMPARE_OP_IMPL(cle_un, <=, stack::native_int, stack::int32, uintptr_t);
    COMPARE_OP_IMPL(cle_un, <=, stack::native_int, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cle_un, <=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cle_un, <=, stack::Ref, stack::Ref, uintptr_t);

    COMPARE_OP_IMPL(cge_un, >=, stack::int32, stack::int32, uint32_t);
    COMPARE_OP_IMPL(cge_un, >=, stack::int32, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cge_un, >=, stack::int64, stack::int64, uint64_t);
    COMPARE_OP_IMPL(cge_un, >=, stack::native_int, stack::int32, uintptr_t);
    COMPARE_OP_IMPL(cge_un, >=, stack::native_int, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cge_un, >=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cge_un, >=, stack::Ref, stack::Ref, uintptr_t);

    COMPARE_OP_IMPL(cgt_un, >, stack::int32, stack::int32, uint32_t);
    COMPARE_OP_IMPL(cgt_un, >, stack::int32, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cgt_un, >, stack::int64, stack::int64, uint64_t);
    COMPARE_OP_IMPL(cgt_un, >, stack::native_int, stack::int32, uintptr_t);
    COMPARE_OP_IMPL(cgt_un, >, stack::native_int, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cgt_un, >, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cgt_un, >, stack::Ref, stack::Ref, uintptr_t);
    // TODO: invalid in ECMA335?, but roslyn generates
    COMPARE_OP_IMPL(cgt_un, >, stack::O, stack::O, uintptr_t);

    COMPARE_OP_IMPL(cne_un, !=, stack::int32, stack::int32, uint32_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::int32, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::int64, stack::int64, uint64_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::native_int, stack::int32, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::native_int, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::native_int, stack::Ref, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::F, stack::F, double);
    COMPARE_OP_IMPL(cne_un, !=, stack::Ref, stack::Ref, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::Ref, stack::native_int, uintptr_t);
    COMPARE_OP_IMPL(cne_un, !=, stack::O, stack::O, uintptr_t);

#undef COMPARE_OP_IMPL

    stack::native_int ldlen(const stack::O &obj);

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
        return stack::details::box_impl<std::decay_t<TFrom>>()(std::forward<TFrom>(value));
    }

    template <class T>
    stack::Ref unbox(const stack::O &obj)
    {
        return stack::details::unbox_impl<T>()(obj);
    }

    template <class T>
    auto unbox_any(const stack::O &obj)
    {
        return stack::details::unbox_any_impl<T>()(obj);
    }

    template <class TFrom>
    stack::Ref ref(TFrom &value) noexcept
    {
        return reinterpret_cast<uintptr_t>(&value);
    }

    template <class TTo, class TFrom>
    auto access(TFrom value) noexcept
    {
        return stack::details::access_impl<TFrom, TTo>()(std::forward<TFrom>(value));
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

    template <class T>
    stack::O isinst(const stack::O &obj) noexcept
    {
        return stack::details::isinst_impl<T>()(obj);
    }

#define LDELEM_IMPL(name, type, value_type)                                              \
    inline value_type ldelem_##name(const stack::O &obj, stack::int32 index)             \
    {                                                                                    \
        using ::System_Private_CorLib::System::SZArray_1;                                \
        check_null_obj_ref(obj);                                                         \
        return stack_from(stack_to<gc_obj_ref<SZArray_1<type>>>(obj)->at(index.value_)); \
    }

    LDELEM_IMPL(i1, ::System_Private_CorLib::System::SByte, stack::int32);
    LDELEM_IMPL(i2, ::System_Private_CorLib::System::Int16, stack::int32);
    LDELEM_IMPL(i4, ::System_Private_CorLib::System::Int32, stack::int32);
    LDELEM_IMPL(i8, ::System_Private_CorLib::System::Int64, stack::int64);
    LDELEM_IMPL(r4, ::System_Private_CorLib::System::Single, stack::F);
    LDELEM_IMPL(r8, ::System_Private_CorLib::System::Double, stack::F);
    LDELEM_IMPL(i, ::System_Private_CorLib::System::IntPtr, stack::native_int);
    LDELEM_IMPL(u1, ::System_Private_CorLib::System::Byte, stack::int32);
    LDELEM_IMPL(u2, ::System_Private_CorLib::System::UInt16, stack::int32);
    LDELEM_IMPL(u4, ::System_Private_CorLib::System::UInt32, stack::int32);
    LDELEM_IMPL(u8, ::System_Private_CorLib::System::UInt64, stack::int64);
    LDELEM_IMPL(u, ::System_Private_CorLib::System::UIntPtr, stack::native_int);

    inline stack::O ldelem_ref(const stack::O &obj, stack::int32 index)
    {
        using ::System_Private_CorLib::System::Object;
        using ::System_Private_CorLib::System::SZArray_1;
        check_null_obj_ref(obj);
        return stack_from(stack_to<gc_obj_ref<SZArray_1<gc_obj_ref<Object>>>>(obj)->at(index.value_));
    }

#undef LDELEM_IMPL

#define STELEM_IMPL(name, type, value_type, cast)                                                       \
    inline void stelem_##name(const stack::O &obj, stack::int32 index, value_type value)                \
    {                                                                                                   \
        using ::System_Private_CorLib::System::SZArray_1;                                               \
        check_null_obj_ref(obj);                                                                        \
        stack_to<gc_obj_ref<SZArray_1<type>>>(obj)->at(index.value_) = static_cast<cast>(value.value_); \
    }

    STELEM_IMPL(i1, ::System_Private_CorLib::System::SByte, stack::int32, int8_t);
    STELEM_IMPL(i2, ::System_Private_CorLib::System::Int16, stack::int32, int16_t);
    STELEM_IMPL(i4, ::System_Private_CorLib::System::Int32, stack::int32, int32_t);
    STELEM_IMPL(i8, ::System_Private_CorLib::System::Int64, stack::int64, int64_t);
    STELEM_IMPL(r4, ::System_Private_CorLib::System::Single, stack::F, float);
    STELEM_IMPL(r8, ::System_Private_CorLib::System::Double, stack::F, double);
    STELEM_IMPL(i, ::System_Private_CorLib::System::IntPtr, stack::native_int, intptr_t);

    inline void stelem_ref(const stack::O &obj, stack::int32 index, stack::O value)
    {
        using ::System_Private_CorLib::System::Object;
        using ::System_Private_CorLib::System::SZArray_1;
        check_null_obj_ref(obj);
        stack_to<gc_obj_ref<SZArray_1<gc_obj_ref<Object>>>>(obj)->at(index.value_) = stack_to<gc_obj_ref<Object>>(value);
    }

#undef STELEM_IMPL

#define LDIND_IMPL(name, type, ret, cast)                                 \
    inline ret ldind_##name(stack::native_int addr)                       \
    {                                                                     \
        check_null_obj_ref(addr);                                         \
        return static_cast<cast>(*reinterpret_cast<type *>(addr.value_)); \
    }                                                                     \
    inline ret ldind_##name(stack::Ref addr)                              \
    {                                                                     \
        check_null_obj_ref(addr);                                         \
        return static_cast<cast>(*reinterpret_cast<type *>(addr.value_)); \
    }

    LDIND_IMPL(i1, int8_t, stack::int32, int32_t);
    LDIND_IMPL(i2, int16_t, stack::int32, int32_t);
    LDIND_IMPL(i4, int32_t, stack::int32, int32_t);
    LDIND_IMPL(i8, int64_t, stack::int64, int64_t);
    LDIND_IMPL(r4, float, stack::F, double);
    LDIND_IMPL(r8, double, stack::F, double);
    LDIND_IMPL(i, intptr_t, stack::native_int, intptr_t);
    LDIND_IMPL(ref, uintptr_t, stack::O, uintptr_t);
    LDIND_IMPL(u1, uint8_t, stack::int32, int32_t);
    LDIND_IMPL(u2, uint16_t, stack::int32, int32_t);
    LDIND_IMPL(u4, uint32_t, stack::int32, int32_t);

#undef LDIND_IMPL

#define STIND_IMPL(name, type, value_type)                                        \
    inline void stind_##name(stack::native_int addr, value_type value)            \
    {                                                                             \
        check_null_obj_ref(addr);                                                 \
        *reinterpret_cast<type *>(addr.value_) = static_cast<type>(value.value_); \
    }                                                                             \
    inline void stind_##name(stack::Ref addr, value_type value)                   \
    {                                                                             \
        check_null_obj_ref(addr);                                                 \
        *reinterpret_cast<type *>(addr.value_) = static_cast<type>(value.value_); \
    }

    STIND_IMPL(i1, int8_t, stack::int32);
    STIND_IMPL(i2, int16_t, stack::int32);
    STIND_IMPL(i4, int32_t, stack::int32);
    STIND_IMPL(i8, int64_t, stack::int64);
    STIND_IMPL(r4, float, stack::F);
    STIND_IMPL(r8, double, stack::F);
    STIND_IMPL(i, intptr_t, stack::native_int);
    STIND_IMPL(ref, uintptr_t, stack::O);

#undef STIND_IMPL

#define CONV_IMPL(name, value_type, ret, med_cast, cast)               \
    inline ret conv_##name(value_type value)                           \
    {                                                                  \
        return static_cast<cast>(static_cast<med_cast>(value.value_)); \
    }

    CONV_IMPL(i1, stack::int32, stack::int32, int8_t, int32_t);
    CONV_IMPL(i1, stack::int64, stack::int32, int8_t, int32_t);
    CONV_IMPL(i1, stack::native_int, stack::int32, int8_t, int32_t);
    CONV_IMPL(i1, stack::F, stack::int32, int8_t, int32_t);

    CONV_IMPL(i2, stack::int32, stack::int32, int16_t, int32_t);
    CONV_IMPL(i2, stack::int64, stack::int32, int16_t, int32_t);
    CONV_IMPL(i2, stack::native_int, stack::int32, int16_t, int32_t);
    CONV_IMPL(i2, stack::F, stack::int32, int16_t, int32_t);

    CONV_IMPL(i4, stack::int32, stack::int32, int32_t, int32_t);
    CONV_IMPL(i4, stack::int64, stack::int32, int32_t, int32_t);
    CONV_IMPL(i4, stack::native_int, stack::int32, int32_t, int32_t);
    CONV_IMPL(i4, stack::F, stack::int32, int32_t, int32_t);

    CONV_IMPL(i8, stack::int32, stack::int64, int64_t, int64_t);
    CONV_IMPL(i8, stack::int64, stack::int64, int64_t, int64_t);
    CONV_IMPL(i8, stack::native_int, stack::int64, int64_t, int64_t);
    CONV_IMPL(i8, stack::F, stack::int64, int64_t, int64_t);
    CONV_IMPL(i8, stack::Ref, stack::int64, int64_t, int64_t);
    CONV_IMPL(i8, stack::O, stack::int64, int64_t, int64_t);

    CONV_IMPL(i, stack::int32, stack::native_int, intptr_t, intptr_t);
    CONV_IMPL(i, stack::int64, stack::native_int, intptr_t, intptr_t);
    CONV_IMPL(i, stack::native_int, stack::native_int, intptr_t, intptr_t);
    CONV_IMPL(i, stack::F, stack::native_int, intptr_t, intptr_t);
    CONV_IMPL(i, stack::Ref, stack::native_int, intptr_t, intptr_t);
    CONV_IMPL(i, stack::O, stack::native_int, intptr_t, intptr_t);

    CONV_IMPL(r4, stack::int32, stack::F, float, double);
    CONV_IMPL(r4, stack::int64, stack::F, float, double);
    CONV_IMPL(r4, stack::native_int, stack::F, float, double);
    CONV_IMPL(r4, stack::F, stack::F, float, double);

    CONV_IMPL(r8, stack::int32, stack::F, double, double);
    CONV_IMPL(r8, stack::int64, stack::F, double, double);
    CONV_IMPL(r8, stack::native_int, stack::F, double, double);
    CONV_IMPL(r8, stack::F, stack::F, double, double);

    CONV_IMPL(u1, stack::int32, stack::int32, uint8_t, int32_t);
    CONV_IMPL(u1, stack::int64, stack::int32, uint8_t, int32_t);
    CONV_IMPL(u1, stack::native_int, stack::int32, uint8_t, int32_t);
    CONV_IMPL(u1, stack::F, stack::int32, uint8_t, int32_t);

    CONV_IMPL(u2, stack::int32, stack::int32, uint16_t, int32_t);
    CONV_IMPL(u2, stack::int64, stack::int32, uint16_t, int32_t);
    CONV_IMPL(u2, stack::native_int, stack::int32, uint16_t, int32_t);
    CONV_IMPL(u2, stack::F, stack::int32, uint16_t, int32_t);

    CONV_IMPL(u4, stack::int32, stack::int32, uint32_t, int32_t);
    CONV_IMPL(u4, stack::int64, stack::int32, uint32_t, int32_t);
    CONV_IMPL(u4, stack::native_int, stack::int32, uint32_t, int32_t);
    CONV_IMPL(u4, stack::F, stack::int32, uint32_t, int32_t);

    CONV_IMPL(u8, stack::int32, stack::int64, uint64_t, int64_t);
    CONV_IMPL(u8, stack::int64, stack::int64, uint64_t, int64_t);
    CONV_IMPL(u8, stack::native_int, stack::int64, uint64_t, int64_t);
    CONV_IMPL(u8, stack::F, stack::int64, uint64_t, int64_t);
    CONV_IMPL(u8, stack::Ref, stack::int64, uint64_t, int64_t);
    CONV_IMPL(u8, stack::O, stack::int64, uint64_t, int64_t);

    CONV_IMPL(u, stack::int32, stack::native_int, uintptr_t, intptr_t);
    CONV_IMPL(u, stack::int64, stack::native_int, uintptr_t, intptr_t);
    CONV_IMPL(u, stack::native_int, stack::native_int, uintptr_t, intptr_t);
    CONV_IMPL(u, stack::F, stack::native_int, uintptr_t, intptr_t);
    CONV_IMPL(u, stack::Ref, stack::native_int, uintptr_t, intptr_t);
    CONV_IMPL(u, stack::O, stack::native_int, uintptr_t, intptr_t);

#undef CONV_IMPL

#define CONV_OVF_IMPL(name, value_type, ret, pre_cast, med_cast, cast)                                 \
    inline ret conv_ovf_##name(value_type value)                                                       \
    {                                                                                                  \
        auto v1 = static_cast<pre_cast>(value.value_);                                                 \
        if (v1 < std::numeric_limits<med_cast>::lowest() || v1 > std::numeric_limits<med_cast>::max()) \
            throw_overflow_exception();                                                                \
        return static_cast<cast>(static_cast<med_cast>(v1));                                           \
    }

    CONV_OVF_IMPL(i1, stack::int32, stack::int32, int32_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1, stack::int64, stack::int32, int64_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1, stack::native_int, stack::int32, intptr_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1, stack::F, stack::int32, double, int8_t, int32_t);

    CONV_OVF_IMPL(i2, stack::int32, stack::int32, int32_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2, stack::int64, stack::int32, int64_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2, stack::native_int, stack::int32, intptr_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2, stack::F, stack::int32, double, int16_t, int32_t);

    CONV_OVF_IMPL(i4, stack::int32, stack::int32, int32_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4, stack::int64, stack::int32, int64_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4, stack::native_int, stack::int32, intptr_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4, stack::F, stack::int32, double, int32_t, int32_t);

    CONV_OVF_IMPL(i8, stack::int32, stack::int64, int32_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8, stack::int64, stack::int64, int64_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8, stack::native_int, stack::int64, intptr_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8, stack::F, stack::int64, double, int64_t, int64_t);
    CONV_OVF_IMPL(i8, stack::Ref, stack::int64, intptr_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8, stack::O, stack::int64, intptr_t, int64_t, int64_t);

    CONV_OVF_IMPL(i, stack::int32, stack::native_int, int32_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i, stack::int64, stack::native_int, int64_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i, stack::native_int, stack::native_int, intptr_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i, stack::F, stack::native_int, double, intptr_t, intptr_t);
    CONV_OVF_IMPL(i, stack::Ref, stack::native_int, intptr_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i, stack::O, stack::native_int, intptr_t, intptr_t, intptr_t);

    CONV_OVF_IMPL(u1, stack::int32, stack::int32, int32_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1, stack::int64, stack::int32, int64_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1, stack::native_int, stack::int32, intptr_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1, stack::F, stack::int32, double, uint8_t, int32_t);

    CONV_OVF_IMPL(u2, stack::int32, stack::int32, int32_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2, stack::int64, stack::int32, int64_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2, stack::native_int, stack::int32, intptr_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2, stack::F, stack::int32, double, uint16_t, int32_t);

    CONV_OVF_IMPL(u4, stack::int32, stack::int32, int32_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4, stack::int64, stack::int32, int64_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4, stack::native_int, stack::int32, intptr_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4, stack::F, stack::int32, double, uint32_t, int32_t);

    CONV_OVF_IMPL(u8, stack::int32, stack::int64, int32_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8, stack::int64, stack::int64, int64_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8, stack::native_int, stack::int64, intptr_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8, stack::F, stack::int64, double, uint64_t, int64_t);
    CONV_OVF_IMPL(u8, stack::Ref, stack::int64, intptr_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8, stack::O, stack::int64, intptr_t, uint64_t, int64_t);

    CONV_OVF_IMPL(u, stack::int32, stack::native_int, int32_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u, stack::int64, stack::native_int, int64_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u, stack::native_int, stack::native_int, intptr_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u, stack::F, stack::native_int, double, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u, stack::Ref, stack::native_int, intptr_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u, stack::O, stack::native_int, intptr_t, uintptr_t, intptr_t);

    CONV_OVF_IMPL(i1_un, stack::int32, stack::int32, uint32_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1_un, stack::int64, stack::int32, uint64_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1_un, stack::native_int, stack::int32, uintptr_t, int8_t, int32_t);
    CONV_OVF_IMPL(i1_un, stack::F, stack::int32, double, int8_t, int32_t);

    CONV_OVF_IMPL(i2_un, stack::int32, stack::int32, uint32_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2_un, stack::int64, stack::int32, uint64_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2_un, stack::native_int, stack::int32, uintptr_t, int16_t, int32_t);
    CONV_OVF_IMPL(i2_un, stack::F, stack::int32, double, int16_t, int32_t);

    CONV_OVF_IMPL(i4_un, stack::int32, stack::int32, uint32_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4_un, stack::int64, stack::int32, uint64_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4_un, stack::native_int, stack::int32, uintptr_t, int32_t, int32_t);
    CONV_OVF_IMPL(i4_un, stack::F, stack::int32, double, int32_t, int32_t);

    CONV_OVF_IMPL(i8_un, stack::int32, stack::int64, uint32_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8_un, stack::int64, stack::int64, uint64_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8_un, stack::native_int, stack::int64, uintptr_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8_un, stack::F, stack::int64, double, int64_t, int64_t);
    CONV_OVF_IMPL(i8_un, stack::Ref, stack::int64, uintptr_t, int64_t, int64_t);
    CONV_OVF_IMPL(i8_un, stack::O, stack::int64, uintptr_t, int64_t, int64_t);

    CONV_OVF_IMPL(i_un, stack::int32, stack::native_int, uint32_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i_un, stack::int64, stack::native_int, uint64_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i_un, stack::native_int, stack::native_int, uintptr_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i_un, stack::F, stack::native_int, double, intptr_t, intptr_t);
    CONV_OVF_IMPL(i_un, stack::Ref, stack::native_int, uintptr_t, intptr_t, intptr_t);
    CONV_OVF_IMPL(i_un, stack::O, stack::native_int, uintptr_t, intptr_t, intptr_t);

    CONV_OVF_IMPL(u1_un, stack::int32, stack::int32, uint32_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1_un, stack::int64, stack::int32, uint64_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1_un, stack::native_int, stack::int32, uintptr_t, uint8_t, int32_t);
    CONV_OVF_IMPL(u1_un, stack::F, stack::int32, double, uint8_t, int32_t);

    CONV_OVF_IMPL(u2_un, stack::int32, stack::int32, uint32_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2_un, stack::int64, stack::int32, uint64_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2_un, stack::native_int, stack::int32, uintptr_t, uint16_t, int32_t);
    CONV_OVF_IMPL(u2_un, stack::F, stack::int32, double, uint16_t, int32_t);

    CONV_OVF_IMPL(u4_un, stack::int32, stack::int32, uint32_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4_un, stack::int64, stack::int32, uint64_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4_un, stack::native_int, stack::int32, uintptr_t, uint32_t, int32_t);
    CONV_OVF_IMPL(u4_un, stack::F, stack::int32, double, uint32_t, int32_t);

    CONV_OVF_IMPL(u8_un, stack::int32, stack::int64, uint32_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8_un, stack::int64, stack::int64, uint64_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8_un, stack::native_int, stack::int64, uintptr_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8_un, stack::F, stack::int64, double, uint64_t, int64_t);
    CONV_OVF_IMPL(u8_un, stack::Ref, stack::int64, uintptr_t, uint64_t, int64_t);
    CONV_OVF_IMPL(u8_un, stack::O, stack::int64, uintptr_t, uint64_t, int64_t);

    CONV_OVF_IMPL(u_un, stack::int32, stack::native_int, uint32_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u_un, stack::int64, stack::native_int, uint64_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u_un, stack::native_int, stack::native_int, uintptr_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u_un, stack::F, stack::native_int, double, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u_un, stack::Ref, stack::native_int, uintptr_t, uintptr_t, intptr_t);
    CONV_OVF_IMPL(u_un, stack::O, stack::native_int, uintptr_t, uintptr_t, intptr_t);

#undef CONV_OVF_IMPL

    template <class T>
    stack::Ref ldelema(const stack::O &obj, stack::int32 index)
    {
        using ::System_Private_CorLib::System::SZArray_1;
        return stack_from(stack_to<gc_obj_ref<SZArray_1<T>>>(obj)->ref_at(index.value_));
    }

    template <class T>
    auto ldobj(const stack::Ref &addr)
    {
        return stack_from(*reinterpret_cast<T *>(addr.value_));
    }

    template <class T, class TStack>
    void stobj(const TStack &src, const stack::Ref &dest)
    {
        *reinterpret_cast<T *>(dest.value_) = stack_to<T>(src);
    }

    [[noreturn]] void throw_(const stack::O &obj);
}

template <class T>
gc_ref<T> unbox_exact(gc_obj_ref<::System_Private_CorLib::System::Object> value)
{
    auto box = value.cast<T>();
    return gc_ref_from_ref(*box);
}
}

namespace System_Private_CorLib
{
template <class T>
void System::ByReference_1<T>::_ctor(::natsu::gc_ref<System::ByReference_1<T>> _this, ::natsu::gc_ref<::natsu::variable_type_t<T>> value)
{
    _this->_value = static_cast<intptr_t>(static_cast<uintptr_t>(value));
}

template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> System::ByReference_1<T>::get_Value(::natsu::gc_ref<System::ByReference_1<T>> _this)
{
    return ::natsu::gc_ref_from_ref(*reinterpret_cast<::natsu::variable_type_t<T> *>(static_cast<intptr_t>(_this->_value)));
}
}
