// natsu clr runtime
#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace System_Private_CorLib
{
namespace System
{
    struct Exception;
    struct String;

    template <class T>
    struct SZArray_1;
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
struct gc_obj_ref;

struct null_gc_obj_ref
{
    constexpr null_gc_obj_ref(std::nullptr_t = nullptr) noexcept
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return 0;
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

    constexpr operator bool() const noexcept
    {
        return true;
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    explicit operator T() const noexcept
    {
        return *ptr_;
    }

    T *operator->() const noexcept
    {
        return ptr_;
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

    template <class U>
    gc_ptr(const gc_ptr<U> &other) noexcept
        : ptr_(reinterpret_cast<T *>(other.ptr_))
    {
    }

    explicit operator uintptr_t() const noexcept
    {
        return reinterpret_cast<uintptr_t>(ptr_);
    }

    operator bool() const noexcept
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

    gc_ptr &operator=(uintptr_t ptr) noexcept
    {
        ptr_ = reinterpret_cast<T *>(ptr);
        return *this;
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

    template <class U>
    gc_obj_ref(gc_obj_ref<U> &&other) noexcept
        : ptr_(reinterpret_cast<T *>(other.ptr_))
    {
        other.ptr_ = nullptr;
    }

    template <class U>
    gc_obj_ref(const gc_obj_ref<U> &other) noexcept
        : ptr_(reinterpret_cast<T *>(other.ptr_))
    {
    }

    T *operator->() const
    {
        assert(ptr_);
        return ptr_;
    }

    T &operator*() const noexcept
    {
        assert(ptr_);
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
            auto vtable = ptr_->header_.vtable_as<typename U::VTable>();
            if (vtable)
                return gc_obj_ref<U>(reinterpret_cast<U *>(ptr_));
        }

        return null;
    }

    template <class U>
    gc_ref<U> unbox()
    {
        assert(ptr_);
        auto ptr = as<U>();
        return gc_ref<U>(*ptr);
    }
};

typedef struct _vtable
{
    virtual ~_vtable() = default;
} vtable_t;

struct object_header
{
    const vtable_t *vtable_;
    uintptr_t length_;

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

struct natsu_exception
{
    template <class T>
    natsu_exception(gc_obj_ref<T> &&exception)
        : exception_(std::move(exception))
    {
    }

    gc_obj_ref<::System_Private_CorLib::System::Exception> exception_;
};

template <class T>
struct static_holder
{
    static T value;
};

template <class T>
T static_holder<T>::value;

gc_obj_ref<object> gc_alloc(const vtable_t &vtable, size_t size);

template <class T>
gc_obj_ref<T> gc_new(size_t size)
{
    auto ptr = gc_alloc(static_holder<typename T::VTable>::value, size);
    return gc_obj_ref<T>(ptr);
}

template <class T>
gc_obj_ref<T> gc_new()
{
    return gc_new<T>(sizeof(T));
}

template <class T>
gc_obj_ref<::System_Private_CorLib::System::SZArray_1<T>> gc_new_array(int length)
{
    using obj_t = ::System_Private_CorLib::System::SZArray_1<T>;
    auto size = sizeof(obj_t) + (size_t)length * sizeof(T);
    auto obj = gc_new<obj_t>(size);
    obj->header_.length_ = length;
    return obj;
}

template <class T, bool IsValueType, class... TArgs>
auto make_object(TArgs... args)
{
    if constexpr (IsValueType)
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

gc_obj_ref<::System_Private_CorLib::System::String> load_string(std::u16string_view string);
std::u16string_view to_string_view(gc_obj_ref<::System_Private_CorLib::System::String> string);
}

#define NATSU_PRIMITIVE_IMPL_BYTE           \
    Byte() = default;                       \
    Byte(uint8_t value) : m_value(value) {} \
    operator uint8_t() const noexcept { return m_value; }

#define NATSU_PRIMITIVE_IMPL_SBYTE          \
    SByte() = default;                      \
    SByte(int8_t value) : m_value(value) {} \
    operator int8_t() const noexcept { return m_value; }

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

#define NATSU_PRIMITIVE_IMPL_UINT32            \
    UInt32() = default;                        \
    UInt32(uint32_t value) : m_value(value) {} \
    operator uint32_t() const noexcept { return m_value; }

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

#define NATSU_PRIMITIVE_IMPL_INTPTR

#define NATSU_PRIMITIVE_IMPL_UINTPTR

#define NATSU_ENUM_IMPL_INT32(name)         \
    name() = default;                       \
    name(int32_t value) : value__(value) {} \
    operator int32_t() const noexcept { return value__; }

#define NATSU_OBJECT_IMPL

#define NATSU_SZARRAY_IMPL                                 \
    T &at(int index)                                       \
    {                                                      \
        assert(index < header_.length_);                   \
        return elements_[index];                           \
    }                                                      \
    ::natsu::gc_ref<T> ref_at(int index)                   \
    {                                                      \
        assert(index < header_.length_);                   \
        return ::natsu::gc_ref_from_ref(elements_[index]); \
    }                                                      \
    T get(int index)                                       \
    {                                                      \
        return at(index);                                  \
    }                                                      \
    void set(int index, T value)                           \
    {                                                      \
        at(index) = value;                                 \
    }                                                      \
    uintptr_t length() const noexcept                      \
    {                                                      \
        return header_.length_;                            \
    }                                                      \
    T elements_[0];
