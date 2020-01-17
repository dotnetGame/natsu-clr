// natsu clr runtime
#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>

using namespace std::string_view_literals;

namespace System_Private_CoreLib
{
namespace System
{
    struct Boolean;
    struct SByte;
    struct Byte;
    struct Char;
    struct UInt16;
    struct Int16;
    struct UInt32;
    struct Int32;
    struct UInt64;
    struct Int64;
    struct Single;
    struct Double;
    struct UIntPtr;
    struct IntPtr;
    struct Exception;
    struct String;
    struct Object;

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

struct clr_exception;

[[noreturn]] void throw_null_ref_exception();
[[noreturn]] void throw_invalid_cast_exception();
[[noreturn]] void throw_index_out_of_range_exception();
[[noreturn]] void throw_overflow_exception();
[[noreturn]] void pure_call();

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

template <class T, size_t N>
constexpr T bit_init(std::array<uint8_t, N> value) noexcept
{
    T t;
    std::memcpy(&t, value.data(), sizeof(t));
    return t;
}

inline constexpr void nop() noexcept
{
}

template <class T>
struct static_holder
{
    static T &get()
    {
        static T value;
        return value;
    }
};

template <class T>
struct vtable_holder
{
    static const constexpr T value = T();

    static constexpr const T &get()
    {
        return value;
    }
};

typedef struct _vtable
{
    uint32_t ElementSize;

    constexpr _vtable()
        : ElementSize(0)
    {
    }

    virtual void dummy() const noexcept {}

    template <class TFunc>
    constexpr void override_vfunc_impl(std::string_view name, TFunc func)
    {
    }
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

template <class T>
struct to_clr_type
{
    using type = T;
};

#define DEFINE_TO_CLR_TYPE(src, dest)                        \
    template <>                                              \
    struct to_clr_type<src>                                  \
    {                                                        \
        using type = ::System_Private_CoreLib::System::dest; \
    };

DEFINE_TO_CLR_TYPE(bool, Boolean);
DEFINE_TO_CLR_TYPE(int8_t, SByte);
DEFINE_TO_CLR_TYPE(uint8_t, Byte);
DEFINE_TO_CLR_TYPE(char16_t, Char);
DEFINE_TO_CLR_TYPE(uint16_t, UInt16);
DEFINE_TO_CLR_TYPE(int16_t, Int16);
DEFINE_TO_CLR_TYPE(uint32_t, UInt32);
DEFINE_TO_CLR_TYPE(int32_t, Int32);
DEFINE_TO_CLR_TYPE(uint64_t, UInt64);
DEFINE_TO_CLR_TYPE(int64_t, Int64);
DEFINE_TO_CLR_TYPE(float, Single);
DEFINE_TO_CLR_TYPE(double, Double);
//DEFINE_TO_CLR_TYPE(uintptr_t, UIntPtr);
//DEFINE_TO_CLR_TYPE(intptr_t, IntPtr);

#undef DEFINE_TO_CLR_TYPE

template <class T>
using to_clr_type_t = typename to_clr_type<T>::type;

template <class T>
constexpr bool is_value_type_v = to_clr_type_t<T>::TypeInfo::IsValueType;

template <class T>
constexpr bool is_enum_v = to_clr_type_t<T>::TypeInfo::IsEnum;

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
gc_ref<T> gc_ref_from_addr(uintptr_t addr)
{
    return gc_ref<T>(*reinterpret_cast<T *>(addr));
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

    T *get() const noexcept
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

    template <class U>
    gc_obj_ref(gc_obj_ref<U> &&other) noexcept
        : ptr_(reinterpret_cast<T *>(other.ptr_))
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
        return ptr_;
    }

    T &operator*() const noexcept
    {
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
            auto vtable = header().template vtable_as<typename natsu::to_clr_type_t<U>::VTable>();
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

    object_header &header() const noexcept
    {
        return *reinterpret_cast<object_header *>(reinterpret_cast<uint8_t *>(ptr_) - sizeof(object_header));
    }
};

template <class T>
class clr_volatile
{
public:
    using value_type = T;

    clr_volatile() noexcept
    {
    }

    clr_volatile(T value) noexcept
        : value_(value)
    {
    }

    clr_volatile(const clr_volatile<T> &other) noexcept
        : value_(other.value_.load(std::memory_order_acquire))
    {
    }

    clr_volatile &operator=(const T &other) noexcept
    {
        value_.store(other, std::memory_order_release);
        return *this;
    }

    T load() const noexcept
    {
        return value_.load(std::memory_order_acquire);
    }

private:
    std::atomic<T> value_;
};

struct clr_exception
{
    template <class T>
    clr_exception(gc_obj_ref<T> exception)
        : exception_(std::move(exception))
    {
    }

    gc_obj_ref<::System_Private_CoreLib::System::Exception> exception_;
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

template <class TBase, class TIFace, bool>
struct vtable_impl;

template <class TBase, class TIFace>
struct vtable_impl<TBase, TIFace, true>
{
    template <class TFunc>
    constexpr void override_vfunc_impl(std::string_view name, TFunc func)
    {
    }
};

template <class TBase, class TIFace>
struct vtable_impl<TBase, TIFace, false> : public TIFace
{
    template <class TFunc>
    constexpr void override_vfunc_impl(std::string_view name, TFunc func)
    {
        TIFace::override_vfunc_impl(name, func);
    }
};

template <class TBase, class TIFace>
using vtable_impl_t = vtable_impl<TBase, TIFace, std::is_base_of_v<TIFace, TBase>>;

template <class TBase, class... TIFaces>
struct vtable_class : public TBase, public vtable_impl_t<TBase, TIFaces>...
{
    template <class TFunc>
    constexpr void override_vfunc(std::string_view name, TFunc func)
    {
        TBase::override_vfunc_impl(name, func);
        int ignore[] = { 0, (vtable_impl_t<TBase, TIFaces>::override_vfunc_impl(name, func), 0)... };
    }
};

template <size_t N>
struct string_literal
{
    int32_t _stringLength;
    std::array<char16_t, N + 1> _firstChar;

    constexpr string_literal(std::u16string_view str)
        : _stringLength((int32_t)N), _firstChar(init_array(str, std::make_index_sequence<N>()))
    {
    }

    template <size_t... I>
    constexpr std::array<char16_t, N + 1> init_array(std::u16string_view str, std::index_sequence<I...>)
    {
        return { str[I]..., 0 };
    }
};

template <class TObject, class TValue>
struct static_object
{
    object_header header_;
    TValue value_;

    constexpr static_object(TValue value)
        : header_({ OBJ_ATTR_NONE, &vtable_holder<typename TObject::VTable>::get() }), value_(value)
    {
    }

    constexpr gc_obj_ref<TObject> get() const noexcept
    {
        return gc_obj_ref<TObject>(reinterpret_cast<TObject *>(const_cast<TValue *>(&value_)));
    }
};
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

#define NATSU_ENUM_IMPL_BYTE(name) \
    name &operator=(int32_t value) \
    {                              \
        value__ = value;           \
        return *this;              \
    }                              \
    constexpr operator uint8_t() const noexcept { return value__; }

#define NATSU_ENUM_IMPL_INT32(name) \
    name &operator=(int32_t value)  \
    {                               \
        value__ = value;            \
        return *this;               \
    }                               \
    constexpr operator int32_t() const noexcept { return value__; }

#define NATSU_OBJECT_IMPL

#define NATSU_PRIMITIVE_OPERATORS_IMPL

#define NATSU_SZARRAY_IMPL                                           \
    constexpr ::natsu::variable_type_t<T> &at(size_t index)          \
    {                                                                \
        return elements_[index];                                     \
    }                                                                \
    constexpr ::natsu::variable_type_t<T> *begin()                   \
    {                                                                \
        return elements_;                                            \
    }                                                                \
    constexpr ::natsu::variable_type_t<T> *end()                     \
    {                                                                \
        return elements_ + length();                                 \
    }                                                                \
    constexpr auto ref_at(size_t index)                              \
    {                                                                \
        return ::natsu::gc_ref_from_ref(elements_[index]);           \
    }                                                                \
    constexpr ::natsu::variable_type_t<T> get(int index)             \
    {                                                                \
        return at(index);                                            \
    }                                                                \
    constexpr void set(int index, ::natsu::variable_type_t<T> value) \
    {                                                                \
        at(index) = value;                                           \
    }                                                                \
    constexpr uintptr_t length() const noexcept                      \
    {                                                                \
        return Length;                                               \
    }                                                                \
    ::natsu::variable_type_t<T> elements_[0];
