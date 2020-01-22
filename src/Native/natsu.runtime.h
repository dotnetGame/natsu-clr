// natsu clr runtime
#pragma once
#include "natsu.typedef.h"
#include <cmath>
#include <limits>
#include <optional>
#include <utility>
#if _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif

using namespace std::string_view_literals;

namespace natsu
{
namespace stack
{
    struct int32
    {
        int32_t value_;

        int32() = default;
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

        int64() = default;
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

        native_int() = default;
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

        F() = default;
        constexpr F(double value)
            : value_(value) {}
    };

    struct Ref
    {
        uintptr_t value_;

        Ref() = default;
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

        O() = default;
        constexpr O(uintptr_t value)
            : value_(value) {}

        constexpr operator bool() const noexcept
        {
            return value_;
        }

        object_header &header() const noexcept
        {
            return *reinterpret_cast<object_header *>(reinterpret_cast<uint8_t *>(value_) - sizeof(object_header));
        }
    };

    static constexpr O null = 0;
}

gc_obj_ref<::System_Private_CoreLib::System::Object> gc_alloc(const vtable_t &vtable, size_t size);

template <class T>
gc_obj_ref<T> gc_new(size_t size)
{
    auto obj = gc_alloc(vtable_holder<typename to_clr_type_t<T>::VTable>::get(), size);
    return obj.template cast<T>();
}

template <class T>
gc_obj_ref<T> gc_new()
{
    return gc_new<T>(sizeof(T));
}

template <class T>
gc_obj_ref<::System_Private_CoreLib::System::SZArray_1<T>> gc_new_array(int32_t length)
{
    using obj_t = ::System_Private_CoreLib::System::SZArray_1<T>;
    auto size = sizeof(obj_t) + (size_t)length.value_ * sizeof(T);
    auto obj = gc_new<obj_t>(size);
    obj->Length = length.value_;
    return obj;
}

gc_obj_ref<::System_Private_CoreLib::System::String> load_string(std::u16string_view string);
std::u16string_view to_string_view(gc_obj_ref<::System_Private_CoreLib::System::String> string);

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
auto make_object_uninit()
{
    if constexpr (is_value_type_v<T>)
    {
        T value;
        return value;
    }
    else
    {
        auto value = gc_new<T>();
        return value;
    }
}

template <class T>
clr_exception make_exception(gc_obj_ref<T> exception)
{
    return { exception };
}

template <class T, class... TArgs>
[[noreturn]] void throw_exception(TArgs &&... args)
{
    throw make_exception(make_object<T>(std::forward<TArgs>(args)...));
}

template <class T, class TCond, class... TArgs>
void check_condition(TCond &&condition, TArgs &&... args)
{
    if (!condition)
        throw make_exception(make_object<T>(std::forward<TArgs>(args)...));
}

inline void check_null_obj_ref(stack::O obj)
{
    if (!obj)
        throw_null_ref_exception();
}

inline void check_null_obj_ref(stack::native_int addr)
{
    if (!addr)
        throw_null_ref_exception();
}

inline void check_null_obj_ref(stack::Ref addr)
{
    if (!addr)
        throw_null_ref_exception();
}

template <class T>
struct runtime_type_holder
{
    static gc_obj_ref<::System_Private_CoreLib::System::RuntimeType> get()
    {
        using namespace ::System_Private_CoreLib::System;
        static auto type = make_object<RuntimeType>(
            reinterpret_cast<intptr_t>(&vtable_holder<typename T::VTable>::get()));
        return type;
    }
};

template <class TCall>
class clr_finally
{
public:
    clr_finally(TCall &&call)
        : call_(std::forward<TCall>(call))
    {
    }

    clr_finally(clr_finally &) = delete;
    clr_finally &operator=(clr_finally &) = delete;

    clr_finally(clr_finally &&other)
        : call_(std::move(other.call_))
    {
        other.call_.reset();
    }

    ~clr_finally()
    {
        if (call_)
            (*call_)();
    }

private:
    std::optional<TCall> call_;
};

template <class TCall>
clr_finally<TCall> make_finally(TCall &&call)
{
    return { std::forward<TCall>(call) };
}
}

namespace natsu
{
namespace stack
{
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

        template <class T>
        struct stack_from_impl<clr_volatile<T>>
        {
            auto operator()(const clr_volatile<T> &value) const noexcept
            {
                return stack_from(value.load());
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CoreLib::System::IntPtr>
        {
            native_int operator()(const ::System_Private_CoreLib::System::IntPtr &value) const noexcept
            {
                return (intptr_t) static_cast<uintptr_t>(value._value);
            }
        };

        template <>
        struct stack_from_impl<::System_Private_CoreLib::System::UIntPtr>
        {
            native_int operator()(const ::System_Private_CoreLib::System::UIntPtr &value) const noexcept
            {
                return (intptr_t) static_cast<uintptr_t>(value._value);
            }
        };

#define DEFINE_STACK_FROM_CAST(From, To, Med, Cast)            \
    template <>                                                \
    struct stack_from_impl<From>                               \
    {                                                          \
        To operator()(const From &value) const noexcept        \
        {                                                      \
            return static_cast<Cast>(static_cast<Med>(value)); \
        }                                                      \
    };

        DEFINE_STACK_FROM_CAST(bool, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(int8_t, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(uint8_t, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(char16_t, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(int16_t, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(uint16_t, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(int32_t, int32, int32_t, int32_t);
        DEFINE_STACK_FROM_CAST(uint32_t, int32, uint32_t, int32_t);
        DEFINE_STACK_FROM_CAST(int64_t, int64, int64_t, int64_t);
        DEFINE_STACK_FROM_CAST(uint64_t, int64, uint64_t, int64_t);
        DEFINE_STACK_FROM_CAST(float, F, float, double);
        DEFINE_STACK_FROM_CAST(double, F, double, double);

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
                    return TTo { static_cast<decltype(TTo::value__)>(value.value_) };
                else
                    return value;
            }
        };

        template <class TTo>
        struct stack_to_impl<O, gc_obj_ref<TTo>>
        {
            constexpr gc_obj_ref<TTo> operator()(const O &value) const noexcept
            {
                return gc_obj_ref<TTo>(reinterpret_cast<TTo *>(value.value_));
            }
        };

        template <class TTo>
        struct stack_to_impl<Ref, TTo>
        {
            gc_ref<TTo> operator()(const Ref &value) const noexcept
            {
                return gc_ref<TTo>(*reinterpret_cast<TTo *>(value.value_));
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

        template <class TFrom, class TTo>
        struct stack_to_impl<TFrom, clr_volatile<TTo>>
        {
            TTo operator()(const TFrom &value) const noexcept
            {
                return stack_to_impl<TFrom, TTo>()(value);
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
        DEFINE_STACK_TO_CAST(int32, bool, bool);
        DEFINE_STACK_TO_CAST(int32, int8_t, int8_t);
        DEFINE_STACK_TO_CAST(int32, uint8_t, uint8_t);
        DEFINE_STACK_TO_CAST(int32, char16_t, char16_t);
        DEFINE_STACK_TO_CAST(int32, int16_t, int16_t);
        DEFINE_STACK_TO_CAST(int32, uint16_t, uint16_t);
        DEFINE_STACK_TO_CAST(int32, int32_t, int32_t);
        DEFINE_STACK_TO_CAST(int32, uint32_t, uint32_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CoreLib::System::IntPtr, intptr_t);
        DEFINE_STACK_TO_CAST(int32, ::System_Private_CoreLib::System::UIntPtr, uintptr_t);

        DEFINE_STACK_TO_CAST(native_int, bool, bool);
        DEFINE_STACK_TO_CAST(native_int, int8_t, int8_t);
        DEFINE_STACK_TO_CAST(native_int, uint8_t, uint8_t);
        DEFINE_STACK_TO_CAST(native_int, char16_t, char16_t);
        DEFINE_STACK_TO_CAST(native_int, int16_t, int16_t);
        DEFINE_STACK_TO_CAST(native_int, uint16_t, uint16_t);
        DEFINE_STACK_TO_CAST(native_int, int32_t, int32_t);
        DEFINE_STACK_TO_CAST(native_int, uint32_t, uint32_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CoreLib::System::IntPtr, intptr_t);
        DEFINE_STACK_TO_CAST(native_int, ::System_Private_CoreLib::System::UIntPtr, uintptr_t);

        DEFINE_STACK_TO_CAST(int64, int64_t, int64_t);
        DEFINE_STACK_TO_CAST(int64, uint64_t, uint64_t);

        DEFINE_STACK_TO_CAST(F, float, float);
        DEFINE_STACK_TO_CAST(F, double, double);

#undef DEFINE_STACK_TO_CAST

        template <class T>
        struct box_impl
        {
            auto operator()(T value)
            {
                auto box = gc_new<T>();
                *box = value;
                return box;
            }
        };

        template <class T>
        struct box_impl<gc_obj_ref<T>>
        {
            gc_obj_ref<T> operator()(const gc_obj_ref<T> &value) const noexcept
            {
                return value;
            }
        };

        template <class T>
        struct box_impl<::System_Private_CoreLib::System::Nullable_1<T>>
        {
            auto operator()(const ::System_Private_CoreLib::System::Nullable_1<T> &value) const noexcept
            {
                if (value.hasValue)
                {
                    auto box = gc_new<T>();
                    box->value__ = value;
                    return box;
                }
                else
                {
                    return natsu::null;
                }
            }
        };

        template <class T>
        struct unbox_impl
        {
            Ref operator()(const O &obj)
            {
                using ::System_Private_CoreLib::System::Object;

                // check_null_obj_ref(obj);
                auto box = stack_to<gc_obj_ref<Object>>(obj).template as<T>();
                if (box)
                    return stack_from(gc_ref_from_ref(*box));
                else
                    throw_invalid_cast_exception();
            }
        };

        template <class T>
        struct unbox_impl<::System_Private_CoreLib::System::Nullable_1<T>>
        {
            Ref operator()(const O &obj)
            {
                using ::System_Private_CoreLib::System::Nullable_1;
                using ::System_Private_CoreLib::System::Object;

                if (obj)
                {
                    auto box = stack_to<gc_obj_ref<Object>>(obj).template as<T>();
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
                using ::System_Private_CoreLib::System::Object;

                // check_null_obj_ref(obj);
                auto box = stack_to<gc_obj_ref<Object>>(obj).template as<T>();
                if (box)
                {
                    if constexpr (natsu::is_value_type_v<T>)
                    {
                        return stack_from(*box);
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
        struct unbox_any_impl<::System_Private_CoreLib::System::Nullable_1<T>>
        {
            ::System_Private_CoreLib::System::Nullable_1<T> operator()(const O &obj)
            {
                using ::System_Private_CoreLib::System::Nullable_1;
                using ::System_Private_CoreLib::System::Object;

                if (obj)
                {
                    auto box = stack_to<gc_obj_ref<Object>>(obj).template as<T>();
                    if (box)
                    {
                        return stack_from(make_object<Nullable_1<T>>(*box));
                    }
                    else
                    {
                        throw_invalid_cast_exception();
                    }
                }
                else
                {
                    return stack_from(Nullable_1<T>());
                }
            }
        };

        template <class TFrom, class TTo>
        struct isinst_impl
        {
            gc_obj_ref<TTo> operator()(gc_obj_ref<TFrom> obj) const noexcept
            {
                if (obj && obj.header().vtable_as<typename natsu::to_clr_type_t<T>::VTable>())
                    return obj;
                return null;
            }
        };

        template <class T>
        struct isinst_impl<::System_Private_CoreLib::System::Nullable_1<T>>
        {
            O operator()(const O &obj) const noexcept
            {
                return isinst_impl<T>()(obj);
            }
        };

        template <class T>
        struct castclass_impl
        {
            O operator()(const O &obj) const noexcept
            {
                // check_null_obj_ref(obj);
                if (obj.header().vtable_as<typename T::VTable>())
                    return obj;
                throw_invalid_cast_exception();
            }
        };

        template <class T>
        struct castclass_impl<::System_Private_CoreLib::System::Nullable_1<T>>
        {
            O operator()(const O &obj) const noexcept
            {
                if (obj)
                {
                    if (obj.header().vtable_as<typename T::VTable>())
                        return obj;
                    throw_invalid_cast_exception();
                }

                return null;
            }
        };

        template <class TFrom, class TTo>
        struct access_impl;

        template <class TFrom, class TTo>
        struct access_impl<TFrom, gc_ref<TTo>>
        {
            gc_ref<TTo> operator()(TFrom &obj) const noexcept
            {
                return gc_ref_from_ref(obj);
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

        template <class T1, class T2>
        struct ceq_impl
        {
            int32_t operator()(const T1 &lhs, const T2 rhs) const noexcept
            {
                return lhs == rhs;
            }
        };

        template <class T1, class T2>
        struct cgt_un_impl
        {
            int32_t operator()(const T1 &lhs, const T2 rhs) const noexcept
            {
                return std::lhs == rhs;
            }
        };
    }
}

namespace ops
{
    template <class T, class... TArgs>
    auto newobj(TArgs... args)
    {
        if constexpr (is_value_type_v<T>)
        {
            T value;
            value._ctor(value, std::forward<TArgs>(args)...);
            return value;
        }
        else if constexpr (std::is_same_v<T, ::System_Private_CoreLib::System::String>)
        {
            auto value = ::System_Private_CoreLib::System::String::_s_Ctor(std::forward<TArgs>(args)...);
            return value;
        }
        else
        {
            auto value = gc_new<T>();
            value->_ctor(value, std::forward<TArgs>(args)...);
            return value;
        }
    }

    template <class TFrom>
    auto box(TFrom &&value) noexcept
    {
        return stack::details::box_impl<std::decay_t<TFrom>>()(std::forward<TFrom>(value));
    }

    template <class TFrom, class TTo>
    auto unbox(TFrom &&obj)
    {
        return stack::details::unbox_impl<TTo>()(obj);
    }

    template <class TFrom, class TTo>
    auto unbox_any(TFrom &&obj)
    {
        return stack::details::unbox_any_impl<T>()(obj);
    }

    template <class TFrom>
    natsu::gc_ref<TFrom> ref(TFrom &value) noexcept
    {
        return natsu::gc_ref_from_ref(&value);
    }

    template <class TFrom>
    void initobj(gc_ref<TFrom> addr) noexcept
    {
        std::memset((void *)addr.value_, 0, sizeof(TFrom));
    }

    template <class TFrom>
    void initobj(intptr_t addr) noexcept
    {
        std::memset((void *)addr, 0, sizeof(TFrom));
    }

    template <class TFrom, class TTo>
    auto isinst(gc_obj_ref<TFrom> obj) noexcept
    {
        return stack::details::isinst_impl<TFrom, TTo>()(obj);
    }

    template <class T>
    stack::O castclass(const stack::O &obj) noexcept
    {
        return stack::details::castclass_impl<T>()(obj);
    }

    template <class T>
    constexpr auto unsign(T value) noexcept
    {
        using unsigned_type = std::make_unsigned<T>::type;
        return static_cast<unsigned_type>(value);
    }

    template <class T>
    ::System_Private_CoreLib::System::RuntimeTypeHandle ldtoken_type()
    {
        return { runtime_type_holder<natsu::to_clr_type_t<T>>::get() };
    }

    [[noreturn]] void throw_(gc_obj_ref<::System_Private_CoreLib::System::Exception> obj);
}

template <class T>
gc_ref<T> unbox_exact(gc_obj_ref<::System_Private_CoreLib::System::Object> value)
{
    auto box = value.cast<T>();
    return gc_ref_from_ref(*box);
}
}

namespace System_Private_CoreLib
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

template <class T>
::natsu::gc_ptr<void> Internal::Runtime::CompilerServices::Unsafe::_s_AsPointer(::natsu::gc_ref<::natsu::variable_type_t<T>> value)
{
    return ::natsu::gc_ptr<void>(value.ptr_);
}

template <class T>
int32_t Internal::Runtime::CompilerServices::Unsafe::_s_SizeOf()
{
    return sizeof(T);
}

template <class T>
::natsu::variable_type_t<T> Internal::Runtime::CompilerServices::Unsafe::_s_As(::natsu::gc_obj_ref<::System_Private_CoreLib::System::Object> value)
{
    return ::natsu::gc_obj_ref<T>(reinterpret_cast<T *>(value.ptr_));
}

template <class TFrom, class TTo>
::natsu::gc_ref<::natsu::variable_type_t<TTo>> Internal::Runtime::CompilerServices::Unsafe::_s_As(::natsu::gc_ref<::natsu::variable_type_t<TFrom>> source)
{
    return ::natsu::gc_ref_from_ref(*reinterpret_cast<::natsu::variable_type_t<TTo> *>(source.ptr_));
}

template <class T>
bool Internal::Runtime::CompilerServices::Unsafe::_s_AreSame(::natsu::gc_ref<::natsu::variable_type_t<T>> left, ::natsu::gc_ref<::natsu::variable_type_t<T>> right)
{
    return left.ptr_ == right.ptr_;
}

template <class T>
bool Internal::Runtime::CompilerServices::Unsafe::_s_IsAddressGreaterThan(::natsu::gc_ref<::natsu::variable_type_t<T>> left, ::natsu::gc_ref<::natsu::variable_type_t<T>> right)
{
    return left.ptr_ > right.ptr_;
}

template <class T>
bool Internal::Runtime::CompilerServices::Unsafe::_s_IsAddressLessThan(::natsu::gc_ref<::natsu::variable_type_t<T>> left, ::natsu::gc_ref<::natsu::variable_type_t<T>> right)
{
    return left.ptr_ < right.ptr_;
}

template <class T>
::natsu::variable_type_t<T> Internal::Runtime::CompilerServices::Unsafe::_s_ReadUnaligned(::natsu::gc_ptr<void> source)
{
    auto obj = natsu::make_object_uninit<T>();
    if constexpr (natsu::is_value_type_v<T>)
        std::memcpy(&obj, source.ptr_, sizeof(T));
    else
        std::memcpy(obj.ptr_, source.ptr_, sizeof(T));
    return obj;
}

template <class T>
::natsu::variable_type_t<T> Internal::Runtime::CompilerServices::Unsafe::_s_ReadUnaligned(::natsu::gc_ref<uint8_t> source)
{
    auto obj = natsu::make_object_uninit<T>();
    if constexpr (natsu::is_value_type_v<T>)
        std::memcpy(&obj, source.ptr_, sizeof(T));
    else
        std::memcpy(obj.ptr_, source.ptr_, sizeof(T));
    return obj;
}

template <class T>
void Internal::Runtime::CompilerServices::Unsafe::_s_WriteUnaligned(::natsu::gc_ptr<void> destination, ::natsu::variable_type_t<T> value)
{
    if constexpr (natsu::is_value_type_v<T>)
        std::memcpy(destination.ptr_, &value, sizeof(T));
    else
        std::memcpy(destination.ptr_, value.ptr_, sizeof(T));
}

template <class T>
void Internal::Runtime::CompilerServices::Unsafe::_s_WriteUnaligned(::natsu::gc_ref<uint8_t> destination, ::natsu::variable_type_t<T> value)
{
    if constexpr (natsu::is_value_type_v<T>)
        std::memcpy(destination.ptr_, &value, sizeof(T));
    else
        std::memcpy(destination.ptr_, value.ptr_, sizeof(T));
}

template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> Internal::Runtime::CompilerServices::Unsafe::_s_AddByteOffset(::natsu::gc_ref<::natsu::variable_type_t<T>> source, ::System_Private_CoreLib::System::IntPtr byteOffset)
{
    return ::natsu::gc_ref_from_addr<::natsu::variable_type_t<T>>(reinterpret_cast<uintptr_t>(source.ptr_) + reinterpret_cast<intptr_t>(byteOffset._value.ptr_));
}

template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> Internal::Runtime::CompilerServices::Unsafe::_s_AsRef(::natsu::gc_ref<::natsu::variable_type_t<T>> source)
{
    return source;
}

template <class T>
System::IntPtr Internal::Runtime::CompilerServices::Unsafe::_s_ByteOffset(::natsu::gc_ref<::natsu::variable_type_t<T>> origin, ::natsu::gc_ref<::natsu::variable_type_t<T>> target)
{
    return reinterpret_cast<intptr_t>(target.ptr_) - reinterpret_cast<intptr_t>(origin.ptr_);
}

template <class T>
bool System::Runtime::CompilerServices::RuntimeHelpers::_s_IsReferenceOrContainsReferences()
{
    return true;
}

template <class T>
bool System::Runtime::CompilerServices::RuntimeHelpers::_s_IsBitwiseEquatable()
{
    return false;
}

template <class T>
::natsu::gc_obj_ref<System::Collections::Generic::EqualityComparer_1<T>> System::Collections::Generic::ComparerHelpers::_s_CreateDefaultEqualityComparer()
{
    return natsu::null;
}
}
