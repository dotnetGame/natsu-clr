// natsu clr runtime
#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace System_Private_CorLib
{
struct _Module_;
namespace Microsoft
{
    namespace CodeAnalysis
    {
        struct EmbeddedAttribute;
    }
}
namespace System
{
    namespace Runtime
    {
        namespace CompilerServices
        {
            struct IsReadOnlyAttribute;
        }
    }
}
namespace System
{
    struct ArgumentException;
}
namespace System
{
    struct ArgumentNullException;
}
namespace System
{
    struct Array;
}
namespace System
{
    struct Attribute;
}
namespace System
{
    struct AttributeUsageAttribute;
}
namespace System
{
    struct CLSCompliantAttribute;
}
namespace System
{
    struct Haha;
}
namespace System
{
    struct Console;
}
namespace System
{
    struct Enum;
}
namespace System
{
    struct Exception;
}
namespace System
{
    struct FlagsAttribute;
}
namespace System
{
    struct HResults;
}
namespace System
{
    struct IComparable;
}
namespace System
{
    template <class T>
    struct IComparable_1;
}
namespace System
{
    struct IConvertible;
}
namespace System
{
    template <class T>
    struct IEquatable_1;
}
namespace System
{
    struct IFormatProvider;
}
namespace System
{
    struct IFormattable;
}
namespace System
{
    struct NotSupportedException;
}
namespace System
{
    struct Object;
}
namespace System
{
    struct ObsoleteAttribute;
}
namespace System
{
    struct SerializableAttribute;
}
namespace System
{
    struct String;
}
namespace System
{
    struct SystemException;
}
namespace System
{
    template <class T>
    struct SZArray_1;
}
namespace System
{
    struct Type;
}
namespace System
{
    struct ValueType;
}
namespace System
{
    namespace Runtime
    {
        namespace Versioning
        {
            struct NonVersionableAttribute;
        }
    }
}
namespace System
{
    namespace Runtime
    {
        namespace Versioning
        {
            struct TargetFrameworkAttribute;
        }
    }
}
namespace System
{
    namespace Runtime
    {
        namespace InteropServices
        {
            struct StructLayoutAttribute;
        }
    }
}
namespace System
{
    namespace Runtime
    {
        namespace CompilerServices
        {
            struct MethodImplAttribute;
        }
    }
}
namespace System
{
    namespace Diagnostics
    {
        struct ConditionalAttribute;
    }
}
namespace System
{
    namespace Diagnostics
    {
        namespace CodeAnalysis
        {
            struct SuppressMessageAttribute;
        }
    }
}
namespace System
{
    namespace ComponentModel
    {
        struct EditorBrowsableAttribute;
    }
}
}

namespace natsu
{
inline constexpr float to_float(uint32_t value) noexcept
{
    return static_cast<const float &>(value);
}

inline constexpr double to_double(uint64_t value) noexcept
{
    return static_cast<const double &>(value);
}

inline constexpr int64_t to_int64(uint64_t value) noexcept
{
    return static_cast<const int64_t &>(value);
}

inline constexpr void nop() noexcept
{
}

template <class T>
struct gc_ptr;

struct null_gc_ptr
{
    constexpr null_gc_ptr()
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return 0;
    }
};

template <class T>
struct gc_ptr
{
    T *ptr_;

    constexpr gc_ptr(null_gc_ptr = {}) noexcept
        : ptr_(nullptr)
    {
    }

    constexpr gc_ptr(T *ptr) noexcept
        : ptr_(ptr)
    {
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
    gc_ptr(gc_ptr<U> &&other) noexcept
        : ptr_(static_cast<T *>(other.ptr_))
    {
        other.ptr_ = nullptr;
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
    gc_ptr(const gc_ptr<U> &other) noexcept
        : ptr_(static_cast<T *>(other.ptr_))
    {
    }

    T *operator->() const
    {
        assert(ptr_);
        return ptr_;
    }

    gc_ptr &operator=(std::nullptr_t) noexcept
    {
        ptr_ = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return ptr_;
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    template <class U>
    gc_ptr<U> as() const noexcept
    {
        return dynamic_cast<U *>(ptr_);
    }

    template <class U>
    U &unbox()
    {
        auto value = dynamic_cast<U *>(ptr_);
        assert(value);
        return *value;
    }
};

struct object_header
{
    uintptr_t length_;
};

uint8_t *gc_alloc(size_t length);

template <class T>
gc_ptr<T> gc_new()
{
    auto ptr = gc_alloc(sizeof(T));
    return reinterpret_cast<T *>(ptr);
}

template <class T>
gc_ptr<::System_Private_CorLib::System::SZArray_1<T>> gc_new_array(int length)
{
    auto size = sizeof(::System_Private_CorLib::System::SZArray_1<T>) + (size_t)length * sizeof(T);
    auto obj = gc_alloc(size);
    auto ptr = reinterpret_cast<::System_Private_CorLib::System::SZArray_1<T> *>(obj);
    ptr->header_.length_ = length;
    return ptr;
}

template <class T, bool IsValueType, class... TArgs>
auto make_object(TArgs... args)
{
    if constexpr (IsValueType)
    {
        T value;
        value._ctor(std::move(args)...);
        return value;
    }
    else
    {
        auto value = gc_new<T>();
        value->_ctor(std::move(args)...);
        return value;
    }
}

template <class T, class U>
constexpr bool operator==(const gc_ptr<T> &lhs, const gc_ptr<U> &rhs) noexcept
{
    return lhs.ptr_ == rhs.ptr_;
}

template <class T, class U>
constexpr bool operator==(const gc_ptr<T> &lhs, U *rhs) noexcept
{
    return lhs.ptr_ == rhs;
}

template <class T, class U>
constexpr bool operator==(U *lhs, const gc_ptr<T> &rhs) noexcept
{
    return lhs == rhs.ptr_;
}

template <class T, class U>
constexpr bool operator!=(const gc_ptr<T> &lhs, const gc_ptr<U> &rhs) noexcept
{
    return lhs.ptr_ != rhs.ptr_;
}

template <class T>
constexpr bool operator==(const gc_ptr<T> &lhs, null_gc_ptr) noexcept
{
    return !lhs.ptr_;
}

template <class T>
constexpr bool operator==(null_gc_ptr, const gc_ptr<T> &rhs) noexcept
{
    return !rhs.ptr_;
}

constexpr null_gc_ptr null;

gc_ptr<::System_Private_CorLib::System::String> load_string(std::u16string_view string);
}

#define NATSU_PRIMITIVE_IMPL_BYTE \
    Byte() = default;             \
    Byte(uint8_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_SBYTE \
    SByte() = default;             \
    SByte(int8_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_BOOLEAN                   \
    Boolean() = default;                               \
    Boolean(bool value) : m_value(value) {}            \
    operator bool() const noexcept { return m_value; } \
    Boolean operator!() const noexcept { return !m_value; }

#define NATSU_PRIMITIVE_IMPL_CHAR \
    Char() = default;             \
    Char(char16_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_UINT16            \
    UInt16() = default;                        \
    UInt16(uint16_t value) : m_value(value) {} \
    operator uint16_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_INT16           \
    Int16() = default;                       \
    Int16(int16_t value) : m_value(value) {} \
    operator int16_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_UINT32 \
    UInt32() = default;             \
    UInt32(uint32_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_INT32           \
    Int32() = default;                       \
    Int32(int32_t value) : m_value(value) {} \
    operator int32_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_UINT64            \
    UInt64() = default;                        \
    UInt64(uint64_t value) : m_value(value) {} \
    operator uint64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_INT64           \
    Int64() = default;                       \
    Int64(int64_t value) : m_value(value) {} \
    operator int64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_SINGLE \
    Single() = default;             \
    Single(float value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_DOUBLE \
    Double() = default;             \
    Double(double value) : m_value(value) {}

#define NATSU_ENUM_IMPL_INT32(name)         \
    name() = default;                       \
    name(int32_t value) : value__(value) {} \
    operator int32_t() const noexcept { return value__; }

#define NATSU_OBJECT_IMPL \
    ::natsu::object_header header_;

#define NATSU_SZARRAY_IMPL               \
    T &at(int index)                     \
    {                                    \
        assert(index < header_.length_); \
        return elements_[index];         \
    }                                    \
    T get(int index)                     \
    {                                    \
        return at(index);                \
    }                                    \
    void set(int index, T value)         \
    {                                    \
        at(index) = value;               \
    }                                    \
    T elements_[0];
