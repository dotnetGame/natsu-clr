#include "System.Private.CorLib.h"
#include <algorithm>

using namespace natsu;
using namespace natsu::stack;
using namespace natsu::ops;
using namespace System_Private_CorLib;
using namespace System_Private_CorLib::System;
using namespace System_Private_CorLib::System::Runtime::CompilerServices;

void InitializeHeap() noexcept;

gc_obj_ref<::System_Private_CorLib::System::String> natsu::load_string(std::u16string_view string)
{
#ifdef WIN32
    InitializeHeap();
#endif
    auto dest = System_Private_CorLib::System::String::_s_FastAllocateString(string.length());
    std::copy(string.begin(), string.end(), &dest->_firstChar);
    (&dest->_firstChar)[string.length()] = 0;
    return dest;
}

std::u16string_view natsu::to_string_view(gc_obj_ref<::System_Private_CorLib::System::String> string)
{
    if (!string)
        return {};
    return { reinterpret_cast<const char16_t *>(&string->_firstChar), (size_t)string->_stringLength };
}

void natsu::throw_null_ref_exception()
{
    throw make_exception(make_object<System_Private_CorLib::System::NullReferenceException>());
}

void natsu::throw_invalid_cast_exception()
{
    throw make_exception(make_object<System_Private_CorLib::System::InvalidCastException>());
}

void natsu::throw_overflow_exception()
{
    throw make_exception(make_object<System_Private_CorLib::System::OverflowException>());
}

native_int ops::ldlen(const O &obj)
{
    check_null_obj_ref(obj);
    return (intptr_t)stack_to<gc_obj_ref<RawSzArrayData>>(obj)->Count;
}

void throw_(const stack::O &obj)
{
    check_null_obj_ref(obj);
    throw make_exception(stack_to<gc_obj_ref<Exception>>(obj));
}
