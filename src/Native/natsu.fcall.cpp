#include "Chino.Kernel.h"
#include <cmath>
#include <cstring>

#ifdef WIN32
#include <Windows.h>
#else
#include "uarths.h"

#define uarths ((volatile uarths_t *)UARTHS_BASE_ADDR)

void uarths_write_byte(uint8_t c)
{
    while (uarths->txdata.full)
        continue;
    uarths->txdata.data = c;
}

void uarths_puts(const char16_t *s)
{
    while (*s)
        uarths_write_byte(*s++);
}
#endif

using namespace natsu;
using namespace System_Private_CorLib;
using namespace System_Private_CorLib::System;
using namespace System_Private_CorLib::System::Diagnostics;
using namespace System_Private_CorLib::System::Runtime::CompilerServices;

Int32 Array::GetLength_System__Int32(gc_obj_ref<Array> _this, Int32 dimension)
{
    if (dimension != 0)
        throw_exception<IndexOutOfRangeException>();
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

Int32 Array::get_Rank_(gc_obj_ref<Array> _this)
{
    return 1;
}

Int32 Array::get_Length_(gc_obj_ref<Array> _this)
{
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

Int64 Array::get_LongLength_(gc_obj_ref<Array> _this)
{
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

Int32 Array::GetUpperBound_System__Int32(gc_obj_ref<Array> _this, Int32 dimension)
{
    return GetLength_System__Int32(_this, dimension);
}

Int32 Array::GetLowerBound_System__Int32(gc_obj_ref<Array> _this, Int32 dimension)
{
    if (dimension != 0)
        throw_exception<IndexOutOfRangeException>();
    return 0;
}

void Array::_s_Copy(gc_obj_ref<Array> sourceArray, Int32 sourceIndex, gc_obj_ref<Array> destinationArray, Int32 destinationIndex, Int32 length, Boolean reliable)
{
    throw_exception<InvalidOperationException>();
}

void Buffer::_s_Memcpy(gc_ptr<Byte> dest, gc_ptr<Byte> src, Int32 len)
{
    std::memcpy(dest, src, len);
}

void Debug::_s_WriteCore(gc_obj_ref<String> message)
{
#ifdef WIN32
    assert(message);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
#else
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
#endif
}

void Debug::_s_WriteLineCore(gc_obj_ref<String> message)
{
#ifdef WIN32
    static const char16_t new_line[] = u"\n";
    assert(message);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), new_line, 1, nullptr, nullptr);
#else
    uarths_puts(&message->_firstChar.m_value);
    uarths_puts(u"\n");
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(::System_Private_CorLib::System::Environment::_s_get_NewLine());
#endif
}

void Debug::_s_FailCore(gc_obj_ref<String> message, gc_obj_ref<String> detailMessage)
{
#ifdef WIN32
    assert(message);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
#else
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
#endif
}

void Buffer::_s_Memmove(gc_ptr<Byte> dest, gc_ptr<Byte> src, UInt64 len)
{
    std::memmove(dest, src, len);
}

gc_obj_ref<Type> Object::GetType_(::natsu::gc_obj_ref<Object> _this)
{
    check_null_obj_ref(_this);
    return ::natsu::null;
}

Char String::get_Chars_System__Int32(gc_obj_ref<String> _this, Int32 index)
{
    if ((uint32_t)index >= (uint32_t)_this->_stringLength)
        throw_exception<IndexOutOfRangeException>();
    return (&_this->_firstChar)[index];
}

Int32 String::get_Length_(gc_obj_ref<String> _this)
{
    return _this->_stringLength;
}

gc_obj_ref<String> String::_s_FastAllocateString(Int32 length)
{
    auto size = sizeof(String) + length * sizeof(Char);
    auto obj = natsu::gc_new<String>(size);
    obj->_stringLength = length;
    (&obj->_firstChar)[length] = 0;
    return obj;
}

Int32 String::_s_wcslen(gc_ptr<Char> ptr)
{
    std::u16string_view sv = reinterpret_cast<const char16_t *>(&ptr->m_value);
    return (int32_t)sv.length();
}

Double Math::_s_Abs(Double value)
{
    return fabs(value.m_value);
}

Single Math::_s_Abs(Single value)
{
    return fabsf(value.m_value);
}

Double Math::_s_Acos(Double value)
{
    return acos(value.m_value);
}

Double Math::_s_Acosh(Double value)
{
    return acosh(value.m_value);
}

Double Math::_s_Asin(Double value)
{
    return asin(value.m_value);
}

Double Math::_s_Asinh(Double value)
{
    return asinh(value.m_value);
}

Double Math::_s_Atan(Double value)
{
    return atan(value.m_value);
}

Double Math::_s_Atan2(Double y, Double x)
{
    return atan2(y.m_value, x.m_value);
}

Double Math::_s_Atanh(Double value)
{
    return atanh(value.m_value);
}

Double Math::_s_Cbrt(Double value)
{
    return cbrt(value.m_value);
}

Double Math::_s_Ceiling(Double value)
{
    return ceil(value.m_value);
}

Double Math::_s_Cos(Double value)
{
    return cos(value.m_value);
}

Double Math::_s_Cosh(Double value)
{
    return cosh(value.m_value);
}

Double Math::_s_Exp(Double value)
{
    return exp(value.m_value);
}

Double Math::_s_Floor(Double value)
{
    return floor(value.m_value);
}

Double Math::_s_Log(Double value)
{
    return log(value.m_value);
}

Double Math::_s_Log10(Double value)
{
    return log10(value.m_value);
}

Double Math::_s_Pow(Double x, Double y)
{
    return pow(x.m_value, y.m_value);
}

Double Math::_s_Sin(Double value)
{
    return sin(value.m_value);
}

Double Math::_s_Sinh(Double value)
{
    return sinh(value.m_value);
}

Double Math::_s_Sqrt(Double value)
{
    return sqrt(value.m_value);
}

Double Math::_s_Tan(Double value)
{
    return tan(value.m_value);
}

Double Math::_s_Tanh(Double value)
{
    return tanh(value.m_value);
}

Double Math::_s_FMod(Double x, Double y)
{
    return fmod(x.m_value, y.m_value);
}

Double Math::_s_ModF(Double x, gc_ptr<Double> y)
{
    return modf(x.m_value, &y->m_value);
}
