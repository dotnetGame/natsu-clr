#include "Chino.Kernel.h"
#include "System.Private.CorLib.h"
#include <cstring>

#ifdef WIN32
#include <Windows.h>
#endif

namespace System_Private_CorLib
{
::natsu::gc_obj_ref<::System_Private_CorLib::System::Type> System::Object::GetType(::natsu::gc_obj_ref<::System_Private_CorLib::System::Object> _this)
{
    return ::natsu::null;
}

::System_Private_CorLib::System::Int32 System::Array::GetLength(::natsu::gc_obj_ref<::System_Private_CorLib::System::Array> _this, ::System_Private_CorLib::System::Int32 dimension)
{
    assert(dimension == 0);
    return _this->header_.length_;
}

::System_Private_CorLib::System::Int32 System::Array::get_Rank(::natsu::gc_obj_ref<::System_Private_CorLib::System::Array> _this)
{
    return 1;
}

::System_Private_CorLib::System::Int32 System::Array::get_Length(::natsu::gc_obj_ref<::System_Private_CorLib::System::Array> _this)
{
    return _this->header_.length_;
}

::System_Private_CorLib::System::Int64 System::Array::get_LongLength(::natsu::gc_obj_ref<::System_Private_CorLib::System::Array> _this)
{
    return _this->header_.length_;
}

::System_Private_CorLib::System::Char System::String::get_Chars(::natsu::gc_obj_ref<::System_Private_CorLib::System::String> _this, ::System_Private_CorLib::System::Int32 index)
{
    assert((uint32_t)index < (uint32_t)_this->_stringLength);
    return (&_this->_firstChar)[index];
}

void System::Diagnostics::Debug::_s_WriteCore(::natsu::gc_obj_ref<::System_Private_CorLib::System::String> message)
{
#ifdef WIN32
    assert(message);
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
#else
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
#endif
}

void System::Buffer::_s_Memcpy(::natsu::gc_ptr<::System_Private_CorLib::System::Byte> dest, ::natsu::gc_ptr<::System_Private_CorLib::System::Byte> src, ::System_Private_CorLib::System::Int32 len)
{
    std::memcpy(dest, src, len);
}

void System::Buffer::_s_Memmove(::natsu::gc_ptr<::System_Private_CorLib::System::Byte> dest, ::natsu::gc_ptr<::System_Private_CorLib::System::Byte> src, ::System_Private_CorLib::System::UInt64 len)
{
    std::memmove(dest, src, len);
}

::System_Private_CorLib::System::Int32 System::String::get_Length(::natsu::gc_obj_ref<::System_Private_CorLib::System::String> _this)
{
    return _this->_stringLength;
}

::natsu::gc_obj_ref<::System_Private_CorLib::System::String> System::String::_s_FastAllocateString(::System_Private_CorLib::System::Int32 length)
{
    auto size = sizeof(System::String) + length * sizeof(System::Char);
    auto obj = natsu::gc_new<System::String>(size);
    obj->_stringLength = length;
    (&obj->_firstChar)[length] = 0;
    return obj;
}
}
