// natsu clr runtime
#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>
#include <cassert>
#include <string_view>

namespace System_Private_CorLib
{
    struct _Module_;
    namespace Microsoft { namespace CodeAnalysis { struct EmbeddedAttribute; } }
    namespace System { namespace Runtime { namespace CompilerServices { struct IsReadOnlyAttribute; } } }
    namespace System { struct ArgumentException; }
    namespace System { struct ArgumentNullException; }
    namespace System { struct Array; }
    namespace System { struct Attribute; }
    namespace System { struct AttributeUsageAttribute; }
    namespace System { struct CLSCompliantAttribute; }
    namespace System { struct Haha; }
    namespace System { struct Console; }
    namespace System { struct Enum; }
    namespace System { struct Exception; }
    namespace System { struct FlagsAttribute; }
    namespace System { struct HResults; }
    namespace System { struct IComparable; }
    namespace System { template <class T> struct IComparable_1; }
    namespace System { struct IConvertible; }
    namespace System { template <class T> struct IEquatable_1; }
    namespace System { struct IFormatProvider; }
    namespace System { struct IFormattable; }
    namespace System { struct NotSupportedException; }
    namespace System { struct Object; }
    namespace System { struct ObsoleteAttribute; }
    namespace System { struct SerializableAttribute; }
    namespace System { struct String; }
    namespace System { struct SystemException; }
    namespace System { struct Type; }
    namespace System { struct ValueType; }
    namespace System { namespace Runtime { namespace Versioning { struct NonVersionableAttribute; } } }
    namespace System { namespace Runtime { namespace Versioning { struct TargetFrameworkAttribute; } } }
    namespace System { namespace Runtime { namespace InteropServices { struct StructLayoutAttribute; } } }
    namespace System { namespace Runtime { namespace CompilerServices { struct MethodImplAttribute; } } }
    namespace System { namespace Diagnostics { struct ConditionalAttribute; } }
    namespace System { namespace Diagnostics { namespace CodeAnalysis { struct SuppressMessageAttribute; } } }
    namespace System { namespace ComponentModel { struct EditorBrowsableAttribute; } }
}

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

        constexpr gc_ptr() noexcept
            : ptr_(nullptr)
        {
        }

        constexpr gc_ptr(T* ptr) noexcept
            : ptr_(ptr)
        {
        }

        template<class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
        gc_ptr(gc_ptr<U>&& other) noexcept
            : ptr_(static_cast<T*>(other.ptr_))
        {
            other.ptr_ = nullptr;
        }

        template<class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
        gc_ptr(const gc_ptr<U>& other) noexcept
            : ptr_(static_cast<T*>(other.ptr_))
        {
        }

        T* operator->() const
        {
            assert(ptr_);
            return ptr_;
        }

        explicit operator bool() const noexcept
        {
            return ptr_;
        }

        explicit operator uintptr_t() const noexcept
        {
            return ptr_;
        }

        template<class U>
        gc_ptr<U> as() const noexcept
        {
            return dynamic_cast<U*>(ptr_);
        }

        template<class U>
        U& unbox()
        {
            auto value = dynamic_cast<U*>(ptr_);
            assert(value);
            return *value;
        }
    };

    template<class T>
    struct sz_array
    {
        sz_array(int length)
            : length_(length)
        {
        }

        T& at(int index)
        {
            assert(index < length_);
            auto base = reinterpret_cast<uint8_t*>(this) + sizeof(sz_array);
            return *(reinterpret_cast<T*>(base) + index);
        }

        T get(int index)
        {
            return at(index);
        }

        void set(int index, T value)
        {
            at(index) = value;
        }

        int length_;
    };

    template<class T>
    gc_ptr<T> gc_new()
    {
        return new T();
    }

    template<class T>
    gc_ptr<sz_array<T>> gc_new_array(int length)
    {
        auto size = sizeof(sz_array) + (size_t)length * sizeof(T);
        auto obj = new uint8_t[size];
        return reinterpret_cast<sz_array<T>*>(obj);
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

    template<class T, class U>
    constexpr bool operator==(const gc_ptr<T>& lhs, const gc_ptr<U>& rhs) noexcept
    {
        return lhs.ptr_ == rhs.ptr_;
    }

    template<class T, class U>
    constexpr bool operator!=(const gc_ptr<T>& lhs, const gc_ptr<U>& rhs) noexcept
    {
        return lhs.ptr_ != rhs.ptr_;
    }

    template<class T>
    constexpr bool operator==(const gc_ptr<T>& lhs, std::nullptr_t) noexcept
    {
        return !lhs.ptr_;
    }

    template<class T>
    constexpr bool operator==(std::nullptr_t, const gc_ptr<T>& rhs) noexcept
    {
        return !rhs.ptr_;
    }

    constexpr gc_ptr<::System_Private_CorLib::System::Object> null;

    gc_ptr<::System_Private_CorLib::System::String> load_string(std::string_view string);
}

#define NATSU_PRIMITIVE_IMPL_BYTE \
Byte() = default; \
Byte(uint8_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_SBYTE \
SByte() = default; \
SByte(int8_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_BOOLEAN \
Boolean() = default; \
Boolean(bool value) : m_value(value) {} \
operator bool() const noexcept { return m_value; } \
Boolean operator!() const noexcept { return !m_value; }

#define NATSU_PRIMITIVE_IMPL_CHAR \
Char() = default; \
Char(char16_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_UINT16 \
UInt16() = default; \
UInt16(uint16_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_INT16 \
Int16() = default; \
Int16(int16_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_UINT32 \
UInt32() = default; \
UInt32(uint32_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_INT32 \
Int32() = default; \
Int32(int32_t value) : m_value(value) {} \
operator int32_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_UINT64 \
UInt64() = default; \
UInt64(uint64_t value) : m_value(value) {} \
operator uint64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_INT64 \
Int64() = default; \
Int64(int64_t value) : m_value(value) {} \
operator int64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_SINGLE \
Single() = default; \
Single(float value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_DOUBLE \
Double() = default; \
Double(double value) : m_value(value) {}