// natsu clr runtime
#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>

using namespace std::string_view_literals;

namespace System_Private_CorLib
{
namespace System
{
    struct Exception;
    struct String;

    template <class T>
    struct SZArray_1;

    template <class T>
    struct Box_1;
}
}

namespace natsu
{
template <class T>
struct gc_obj_ref;

template <class T>
struct gc_ref;

struct natsu_exception;

[[noreturn]] void throw_null_ref_exception();

template <class T>
void check_null_obj_ref(gc_obj_ref<T> obj)
{
    if (!obj)
        throw_null_ref_exception();
}

template <class T>
void check_null_obj_ref(gc_ref<T> obj)
{
}

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
struct static_holder
{
    static T value;
};

template <class T>
T static_holder<T>::value;

typedef struct _vtable
{
    virtual ~_vtable() = default;
} vtable_t;

enum object_attributes
{
    OBJ_ATTR_NONE
};

struct object_header
{
    object_attributes attributes_;
    const vtable_t *vtable_;

    template <class TVTable>
    const TVTable *vtable() const noexcept
    {
        return static_cast<const TVTable *>(vtable_);
    }

    template <class TVTable>
    const TVTable *vtable_as() const noexcept
    {
        return dynamic_cast<const TVTable *>(vtable_);
    }
};

struct object
{
    object_header header_;
};

template <class T>
struct is_value_type
{
    static constexpr bool value = T::TypeInfo::IsValueType;
};

template <class T>
constexpr bool is_value_type_v = is_value_type<T>::value;

template <class T, bool IsValueType>
struct variable_type;

template <class T>
struct variable_type<T, true>
{
    using type = T;
};

template <class T>
struct variable_type<T, false>
{
    using type = gc_obj_ref<T>;
};

template <class T>
using variable_type_t = typename variable_type<T, is_value_type_v<T>>::type;

struct null_gc_obj_ref
{
    constexpr null_gc_obj_ref(std::nullptr_t = nullptr) noexcept
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return 0;
    }

    explicit operator bool() const noexcept
    {
        return false;
    }
};

constexpr null_gc_obj_ref null;

template <class T>
struct gc_ref
{
    T *ptr_;

    constexpr gc_ref() noexcept
        : ptr_(nullptr)
    {
    }

    constexpr gc_ref(T &ptr) noexcept
        : ptr_(&ptr)
    {
    }

    explicit constexpr gc_ref(uintptr_t ptr) noexcept
        : ptr_(reinterpret_cast<T *>(ptr))
    {
    }

    explicit constexpr operator bool() const noexcept
    {
        return true;
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    T *operator->() const noexcept
    {
        return ptr_;
    }

    T &operator*() const noexcept
    {
        return *ptr_;
    }

    gc_ref &operator=(uintptr_t ptr) noexcept
    {
        ptr_ = reinterpret_cast<T *>(ptr);
        return *this;
    }
};

template <class T>
gc_ref<T> gc_ref_from_ref(T &ref)
{
    return gc_ref<T>(ref);
}

template <class T>
struct gc_ptr
{
    T *ptr_;

    constexpr gc_ptr() noexcept
        : ptr_(nullptr)
    {
    }

    constexpr gc_ptr(T *ptr) noexcept
        : ptr_(ptr)
    {
    }

    explicit constexpr gc_ptr(uintptr_t ptr) noexcept
        : ptr_(reinterpret_cast<T *>(ptr))
    {
    }

    template <class U>
    gc_ptr(const gc_ptr<U> &other) noexcept
        : ptr_(reinterpret_cast<T *>(other.ptr_))
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    explicit operator bool() const noexcept
    {
        return ptr_;
    }

    operator T *() const noexcept
    {
        return ptr_;
    }

    T *operator->() const noexcept
    {
        return ptr_;
    }

    T &operator*() const noexcept
    {
        return *ptr_;
    }

    template <class TOffset>
    gc_ptr operator+(TOffset offset) noexcept
    {
        auto new_ptr = *this;
        new_ptr.ptr_ = reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(new_ptr.ptr_) + offset);
        return new_ptr;
    }
};

template <>
struct gc_ptr<void>
{
    void *ptr_;

    constexpr gc_ptr() noexcept
        : ptr_(nullptr)
    {
    }

    constexpr gc_ptr(void *ptr) noexcept
        : ptr_(ptr)
    {
    }

    explicit gc_ptr(uintptr_t ptr) noexcept
        : ptr_(reinterpret_cast<void *>(ptr))
    {
    }

    template <class U>
    gc_ptr(const gc_ptr<U> &other) noexcept
        : ptr_(reinterpret_cast<void *>(other.ptr_))
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    operator void *() const noexcept
    {
        return ptr_;
    }

    void *operator->() const noexcept
    {
        return ptr_;
    }

    gc_ptr &operator=(uintptr_t ptr) noexcept
    {
        ptr_ = reinterpret_cast<void *>(ptr);
        return *this;
    }

    template <class TOffset>
    gc_ptr operator+(TOffset offset) noexcept
    {
        auto new_ptr = *this;
        new_ptr.ptr_ = reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(new_ptr.ptr_) + offset);
        return new_ptr;
    }
};

template <class T>
struct gc_obj_ref
{
    T *ptr_;

    constexpr gc_obj_ref(null_gc_obj_ref = {}) noexcept
        : ptr_(nullptr)
    {
    }

    explicit constexpr gc_obj_ref(T *ptr) noexcept
        : ptr_(ptr)
    {
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    gc_obj_ref(gc_obj_ref<U> &&other) noexcept
        : ptr_(static_cast<T *>(other.ptr_))
    {
        other.ptr_ = nullptr;
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    gc_obj_ref(const gc_obj_ref<U> &other) noexcept
        : ptr_(static_cast<T *>(other.ptr_))
    {
    }

    T *operator->() const
    {
        check_null_obj_ref(*this);
        return ptr_;
    }

    T &operator*() const noexcept
    {
        check_null_obj_ref(*this);
        return *ptr_;
    }

    gc_obj_ref &operator=(std::nullptr_t) noexcept
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
    gc_obj_ref<U> as() const noexcept
    {
        if (ptr_)
        {
            auto vtable = ptr_->header_.template vtable_as<typename U::VTable>();
            if (vtable)
                return gc_obj_ref<U>(reinterpret_cast<U *>(ptr_));
        }

        return null;
    }

    template <class U>
    gc_obj_ref<U> cast() const noexcept
    {
        return gc_obj_ref<U>(reinterpret_cast<U *>(ptr_));
    }
};

struct natsu_exception
{
    template <class T>
    natsu_exception(gc_obj_ref<T> &&exception)
        : exception_(std::move(exception))
    {
    }

    gc_obj_ref<::System_Private_CorLib::System::Exception> exception_;
};

template <class T, class U>
constexpr bool operator==(const gc_obj_ref<T> &lhs, const gc_obj_ref<U> &rhs) noexcept
{
    return lhs.ptr_ == rhs.ptr_;
}

template <class T, class U>
constexpr bool operator==(const gc_obj_ref<T> &lhs, U *rhs) noexcept
{
    return lhs.ptr_ == rhs;
}

template <class T, class U>
constexpr bool operator==(U *lhs, const gc_obj_ref<T> &rhs) noexcept
{
    return lhs == rhs.ptr_;
}

template <class T, class U>
constexpr bool operator!=(const gc_obj_ref<T> &lhs, const gc_obj_ref<U> &rhs) noexcept
{
    return lhs.ptr_ != rhs.ptr_;
}

template <class T>
constexpr bool operator==(const gc_obj_ref<T> &lhs, null_gc_obj_ref) noexcept
{
    return !lhs.ptr_;
}

template <class T>
constexpr bool operator==(null_gc_obj_ref, const gc_obj_ref<T> &rhs) noexcept
{
    return !rhs.ptr_;
}
}

#define NATSU_PRIMITIVE_IMPL_BYTE                     \
    Byte() = default;                                 \
    constexpr Byte(uint8_t value) : m_value(value) {} \
    constexpr operator uint8_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_SBYTE                    \
    SByte() = default;                                \
    constexpr SByte(int8_t value) : m_value(value) {} \
    constexpr operator int8_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_BOOLEAN                             \
    Boolean() = default;                                         \
    constexpr Boolean(bool value) : m_value(value) {}            \
    constexpr operator bool() const noexcept { return m_value; } \
    constexpr Boolean operator!() const noexcept { return !m_value; }

#define NATSU_PRIMITIVE_IMPL_CHAR \
    Char() = default;             \
    constexpr Char(char16_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_UINT16                      \
    UInt16() = default;                                  \
    constexpr UInt16(uint16_t value) : m_value(value) {} \
    constexpr operator uint16_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_INT16                     \
    Int16() = default;                                 \
    constexpr Int16(int16_t value) : m_value(value) {} \
    constexpr operator int16_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_UINT32 \
    UInt32() = default;             \
    constexpr UInt32(uint32_t value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_INT32                     \
    Int32() = default;                                 \
    constexpr Int32(int32_t value) : m_value(value) {} \
    constexpr operator int32_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_UINT64                      \
    UInt64() = default;                                  \
    constexpr UInt64(uint64_t value) : m_value(value) {} \
    constexpr operator uint64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_INT64                     \
    Int64() = default;                                 \
    constexpr Int64(int64_t value) : m_value(value) {} \
    constexpr operator int64_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_SINGLE \
    Single() = default;             \
    constexpr Single(float value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_DOUBLE \
    Double() = default;             \
    constexpr Double(double value) : m_value(value) {}

#define NATSU_PRIMITIVE_IMPL_INTPTR                      \
    IntPtr() = default;                                  \
    IntPtr(intptr_t value) : _value((uintptr_t)value) {} \
    operator intptr_t() const noexcept { return (uintptr_t)_value; }

#define NATSU_PRIMITIVE_IMPL_UINTPTR                       \
    UIntPtr() = default;                                   \
    UIntPtr(uintptr_t value) : _value((uintptr_t)value) {} \
    operator uintptr_t() const noexcept { return (uintptr_t)_value; }

#define NATSU_ENUM_IMPL_INT32(name)                   \
    name() = default;                                 \
    constexpr name(int32_t value) : value__(value) {} \
    constexpr operator int32_t() const noexcept { return value__; }

#define NATSU_OBJECT_IMPL

#define NATSU_PRIMITIVE_OPERATORS_IMPL

#define NATSU_SZARRAY_IMPL                                                                         \
    T &at(int index)                                                                               \
    {                                                                                              \
        if ((uint32_t)index >= Length)                                                             \
            ::natsu::throw_exception<::System_Private_CorLib::System::IndexOutOfRangeException>(); \
        return elements_[index];                                                                   \
    }                                                                                              \
    ::natsu::gc_ref<T> ref_at(int index)                                                           \
    {                                                                                              \
        if ((uint32_t)index >= Length)                                                             \
            ::natsu::throw_exception<::System_Private_CorLib::System::IndexOutOfRangeException>(); \
        return ::natsu::gc_ref_from_ref(elements_[index]);                                         \
    }                                                                                              \
    T get(int index)                                                                               \
    {                                                                                              \
        return at(index);                                                                          \
    }                                                                                              \
    void set(int index, T value)                                                                   \
    {                                                                                              \
        at(index) = value;                                                                         \
    }                                                                                              \
    T elements_[0];
