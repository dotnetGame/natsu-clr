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
    auto size = sizeof(::System_Private_CorLib::System::String) + length * sizeof(::System_Private_CorLib::System::Char);
    auto obj = natsu::gc_alloc(size);
    auto ptr = reinterpret_cast<::System_Private_CorLib::System::String *>(obj);
    ptr->_stringLength = length;
    (&ptr->_firstChar)[length] = 0;
    return ::natsu::gc_obj_ref<::System_Private_CorLib::System::String>(ptr);
}
}
