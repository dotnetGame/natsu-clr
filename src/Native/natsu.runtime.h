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

#define natsu_alloca(size) reinterpret_cast<uintptr_t>(alloca(size))

namespace natsu
{
template <class T>
constexpr ::System_Private_CoreLib::System::IntPtr operator-(::System_Private_CoreLib::System::IntPtr lhs, gc_ptr<T> rhs) noexcept
{
    return (intptr_t)((intptr_t)lhs - (intptr_t)rhs.ptr_);
}

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

template <class T>
auto &vtable() noexcept
{
    return vtable_holder<typename T::VTable>::get();
}

template <class T>
auto &vtable(gc_obj_ref<> obj) noexcept
{
    check_null_obj_ref(obj);
    return *static_cast<const typename T::VTable *>(obj.header().vtable_);
}

gc_obj_ref<::System_Private_CoreLib::System::Object> gc_alloc(const clr_vtable &vtable, size_t size);

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
    auto size = sizeof(obj_t) + (size_t)length * sizeof(T);
    auto obj = gc_new<obj_t>(size);
    obj->Length = length;
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
        struct box_impl
        {
            gc_obj_ref<> operator()(T value)
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
            gc_obj_ref<> operator()(const ::System_Private_CoreLib::System::Nullable_1<T> &value) const noexcept
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

        template <class TFrom, class TTo>
        struct cast_impl
        {
            TTo operator()(TFrom &&value) const noexcept
            {
                if constexpr (is_enum_v<TTo>)
                {
                    TTo e;
                    e.value__ = static_cast<decltype(e.value__)>(value);
                    return e;
                }
                else
                {
                    return value;
                }
            }
        };

        // ECMA-335 III.4.6
        template <class TFrom, class TTo>
        struct isinst_impl
        {
            gc_obj_ref<TTo> operator()(const gc_obj_ref<TFrom> &obj) const noexcept
            {
                return obj.template as<TTo>();
            }
        };

        // ECMA-335 III.4.6
        template <class TFrom, class TTo>
        struct isinst_impl<TFrom, ::System_Private_CoreLib::System::Nullable_1<TTo>>
        {
            gc_obj_ref<TTo> operator()(const gc_obj_ref<TFrom> &obj) const noexcept
            {
                return isinst_impl<TFrom, TTo>()(obj);
            }
        };

        // ECMA-335 III.4.3
        template <class TFrom, class TTo>
        struct castclass_impl
        {
            gc_obj_ref<TTo> operator()(const gc_obj_ref<TFrom> &obj) const
            {
                if (!obj)
                    return nullptr;
                if (auto new_obj = obj.template as<TTo>())
                    return new_obj;
                throw_invalid_cast_exception();
            }
        };

        // ECMA-335 III.4.3
        // If typeTok is a nullable type, Nullable<T>, it is interpreted as ¡°boxed¡± T.
        template <class TFrom, class TTo>
        struct castclass_impl<TFrom, ::System_Private_CoreLib::System::Nullable_1<TTo>>
        {
            gc_obj_ref<TTo> operator()(const gc_obj_ref<TFrom> &obj) const
            {
                return castclass_impl<TFrom, TTo>()(obj);
            }
        };

        // ECMA-335 III.4.32
        template <class TFrom, class TTo>
        struct unbox_impl
        {
            gc_ref<TTo> operator()(const gc_obj_ref<TFrom> &obj) const
            {
                check_null_obj_ref(obj);
                if (auto box = obj.template as<TTo>())
                    return *box;
                else
                    throw_invalid_cast_exception();
            }
        };

        // ECMA-335 III.4.32
        template <class TFrom, class TTo>
        struct unbox_impl<TFrom, ::System_Private_CoreLib::System::Nullable_1<TTo>>
        {
            using nullable_t = ::System_Private_CoreLib::System::Nullable_1<TTo>;

            gc_ref<nullable_t> operator()(const gc_obj_ref<TFrom> &obj) const
            {
                if (obj)
                {
                    if (auto box = obj.template as<TTo>(obj))
                    {
                        auto new_nullable = ops::newobj<nullable_t>(*box);
                        return new_nullable;
                    }
                    else
                    {
                        throw_invalid_cast_exception();
                    }
                }
                else
                {
                    auto new_nullable = gc_new<nullable_t>();
                    return new_nullable;
                }
            }
        };

        // ECMA-335 III.4.33
        template <class TFrom, class TTo>
        struct unbox_any_impl
        {
            auto operator()(const gc_obj_ref<TFrom> &obj)
            {
                if constexpr (is_value_type_v<TTo>)
                {
                    // System.NullReferenceException is thrown if obj is null and typeTok is a non-nullable value type (Partition I.8.2.4).
                    check_null_obj_ref(obj);
                    if (auto box = obj.template as<TTo>())
                        return *box;
                    else
                        throw_invalid_cast_exception();
                }
                else
                {
                    return castclass_impl<TFrom, TTo>()(obj);
                }
            }
        };

        // ECMA-335 III.4.33
        template <class TFrom, class TTo>
        struct unbox_any_impl<TFrom, ::System_Private_CoreLib::System::Nullable_1<TTo>>
        {
            using nullable_t = ::System_Private_CoreLib::System::Nullable_1<TTo>;

            nullable_t operator()(const gc_obj_ref<TFrom> &obj)
            {
                if (!obj)
                {
                    nullable_t value;
                    ops::initobj(ops::ref(value));
                    return value;
                }

                if (auto box = obj.template as<TTo>())
                    return ops::newobj<nullable_t>(*box);
                else
                    throw_invalid_cast_exception();
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

        template <class T>
        struct ref_impl
        {
            constexpr gc_ref<T> operator()(T &ref) const noexcept
            {
                return ref;
            }
        };

        template <class T>
        struct ref_impl<clr_volatile<T>>
        {
            gc_ref<T> operator()(clr_volatile<T> &ref) const noexcept
            {
                return gc_ref<T>(reinterpret_cast<T &>(ref));
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
    auto box(TFrom value) noexcept
    {
        return stack::details::box_impl<std::decay_t<TFrom>>()(std::forward<TFrom>(value));
    }

    template <class TTo, class TFrom>
    auto cast(TFrom value) noexcept
    {
        return stack::details::cast_impl<std::decay_t<TFrom>, TTo>()(std::forward<TFrom>(value));
    }

    template <class T>
    auto ref(T &value) noexcept
    {
        return stack::details::ref_impl<T>()(value);
    }

    template <class TFrom>
    void initobj(gc_ref<TFrom> addr) noexcept
    {
        std::memset((void *)addr.ptr_, 0, sizeof(TFrom));
    }

    template <class TFrom>
    void initobj(intptr_t addr) noexcept
    {
        std::memset((void *)addr, 0, sizeof(TFrom));
    }

    // ECMA-335 III.4.3
    template <class TTo, class TFrom>
    auto castclass(const gc_obj_ref<TFrom> &obj) noexcept
    {
        return stack::details::castclass_impl<TFrom, TTo>()(obj);
    }

    // ECMA-335 III.4.6
    template <class TTo, class TFrom>
    auto isinst(const gc_obj_ref<TFrom> &obj) noexcept
    {
        return stack::details::isinst_impl<TFrom, TTo>()(obj);
    }

    // ECMA-335 III.4.32
    template <class TTo, class TFrom>
    auto unbox(const gc_obj_ref<TFrom> &obj)
    {
        return stack::details::unbox_impl<TFrom, TTo>()(obj);
    }

    // ECMA-335 III.4.33
    template <class TTo, class TFrom>
    auto unbox_any(const gc_obj_ref<TFrom> &obj)
    {
        return stack::details::unbox_any_impl<TFrom, TTo>()(obj);
    }

    template <class T>
    ::System_Private_CoreLib::System::IntPtr ldftn(T func)
    {
        return reinterpret_cast<intptr_t>(func);
    }

    template <class T>
    T ldind(uintptr_t address)
    {
        if (!address)
            throw_null_ref_exception();
        return *reinterpret_cast<const T *>(address);
    }

    template <class TTo, class TFrom>
    TTo ldind(const gc_ptr<TFrom> &address)
    {
        check_null_obj_ref(address);
        return *reinterpret_cast<const TTo *>(address.get());
    }

    template <class TTo, class TFrom>
    TTo ldind(const gc_ref<TFrom> &address)
    {
        check_null_obj_ref(address);
        return *reinterpret_cast<const TTo *>(address.get());
    }

    template <class TTo, class TFrom>
    void stind(gc_ptr<TFrom> &address, TTo value)
    {
        check_null_obj_ref(address);
        *reinterpret_cast<TTo *>(address.get()) = value;
    }

    template <class TTo, class TFrom>
    void stind(gc_ref<TFrom> &address, TTo value)
    {
        check_null_obj_ref(address);
        *reinterpret_cast<TTo *>(address.get()) = value;
    }

    template <class T>
    ::System_Private_CoreLib::System::RuntimeTypeHandle ldtoken_type()
    {
        ::System_Private_CoreLib::System::RuntimeTypeHandle handle;
        handle._runtimeType = runtime_type_holder<natsu::to_clr_type_t<T>>::get();
        return handle;
    }

    [[noreturn]] void throw_(gc_obj_ref<::System_Private_CoreLib::System::Exception> obj);

    template <class T>
    constexpr auto unsign(T value) noexcept
    {
        if constexpr (is_enum_v<T>)
        {
            using unsigned_type_e = typename std::make_unsigned<decltype(value.value__)>::type;
            return static_cast<unsigned_type_e>(value.value__);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return value;
        }
        else if constexpr (std::is_same_v<T, ::System_Private_CoreLib::System::IntPtr>)
        {
            return (uintptr_t)value;
        }
        else
        {
            using unsigned_type = typename std::make_unsigned<T>::type;
            return static_cast<unsigned_type>(value);
        }
    }
}

template <class T>
gc_ref<T> unbox_exact(gc_obj_ref<::System_Private_CoreLib::System::Object> value)
{
    auto box = value.cast<T>();
    return *box;
}
}

namespace System_Private_CoreLib
{
namespace details
{
    template <class T>
    struct default_comparer_impl
    {
        ::natsu::gc_obj_ref<System::Collections::Generic::Comparer_1<T>> operator()() const
        {
            using namespace System::Collections::Generic;

            // If T implements IComparable<T> return a GenericComparer<T>
            if constexpr (natsu::is_convertible_v<T, IComparable_1<T>>)
                return natsu::make_object<GenericComparer_1<T>>();
            else if constexpr (natsu::is_enum_v<T>)
                return natsu::make_object<EnumComparer_1<T>>();
            else
                return nullptr;
        }
    };

    // Nullable does not implement IComparable<T?> directly because that would add an extra interface call per comparison.
    // Instead, it relies on Comparer<T?>.Default to specialize for nullables and do the lifted comparisons if T implements IComparable.
    template <class T>
    struct default_comparer_impl<System::Nullable_1<T>>
    {
        ::natsu::gc_obj_ref<System::Collections::Generic::Comparer_1<System::Nullable_1<T>>> operator()() const
        {
            using namespace System::Collections::Generic;

            return default_comparer_impl<T>()();
        }
    };

    template <class T>
    struct default_equality_comparer_impl
    {
        ::natsu::gc_obj_ref<System::Collections::Generic::EqualityComparer_1<T>> operator()() const
        {
            using namespace System::Collections::Generic;

            // Specialize for byte so Array.IndexOf is faster.
            if constexpr (std::is_same_v<T, uint8_t>)
                return natsu::make_object<ByteEqualityComparer>();
            else if constexpr (natsu::is_enum_v<T>)
                return natsu::make_object<EnumEqualityComparer_1<T>>();
            // If T implements IEquatable<T> return a GenericEqualityComparer<T>
            else if constexpr (natsu::is_convertible_v<T, ::System_Private_CoreLib::System::IEquatable_1<T>>)
                return natsu::make_object<GenericEqualityComparer_1<T>>();
            else
                return natsu::make_object<ObjectEqualityComparer_1<T>>();
        }
    };

    // Nullable does not implement IEquatable<T?> directly because that would add an extra interface call per comparison.
    // Instead, it relies on EqualityComparer<T?>.Default to specialize for nullables and do the lifted comparisons if T implements IEquatable.
    template <class T>
    struct default_equality_comparer_impl<System::Nullable_1<T>>
    {
        ::natsu::gc_obj_ref<System::Collections::Generic::EqualityComparer_1<System::Nullable_1<T>>> operator()() const
        {
            using namespace System::Collections::Generic;

            return default_equality_comparer_impl<T>()();
        }
    };
}

template <class T>
::natsu::variable_type_t<T> System::Activator::_s_CreateInstance()
{
    if constexpr (::natsu::has_default_ctor_v<T>)
        return ::natsu::make_object<T>();
    else
        ::natsu::throw_exception<System::MissingMethodException>();
}

template <class T>
void System::ByReference_1<T>::_ctor(::natsu::gc_ref<System::ByReference_1<T>> _this, ::natsu::gc_ref<::natsu::variable_type_t<T>> value)
{
    _this->_value = reinterpret_cast<intptr_t>(value.get());
}

template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> System::ByReference_1<T>::get_Value(::natsu::gc_ref<System::ByReference_1<T>> _this)
{
    return *reinterpret_cast<::natsu::variable_type_t<T> *>(static_cast<intptr_t>(_this->_value));
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
    return *reinterpret_cast<::natsu::variable_type_t<TTo> *>(source.ptr_);
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
bool System::Runtime::CompilerServices::RuntimeHelpers::_s_EnumEquals(::natsu::variable_type_t<T> x, ::natsu::variable_type_t<T> y)
{
    return x.value__ == y.value__;
}

template <class T>
::natsu::gc_obj_ref<System::Collections::Generic::EqualityComparer_1<T>> System::Collections::Generic::ComparerHelpers::_s_CreateDefaultEqualityComparer()
{
    using namespace System::Collections::Generic;

    return details::default_equality_comparer_impl<T>()();
}
}
