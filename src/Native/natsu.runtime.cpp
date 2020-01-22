#include "System.Private.CoreLib.h"
#include <algorithm>
#include <stdexcept>

using namespace natsu;
using namespace natsu::stack;
using namespace natsu::ops;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Runtime::CompilerServices;

void InitializeHeap() noexcept;

gc_obj_ref<String> natsu::load_string(std::u16string_view string)
{
    auto dest = String::_s_FastAllocateString(string.length());
    std::copy(string.begin(), string.end(), &dest->_firstChar);
    (&dest->_firstChar)[string.length()] = 0;
    return dest;
}

std::u16string_view natsu::to_string_view(gc_obj_ref<String> string)
{
    if (!string)
        return {};
    return { reinterpret_cast<const char16_t *>(&string->_firstChar), (size_t)string->_stringLength };
}

void natsu::throw_null_ref_exception()
{
    throw make_exception(make_object<NullReferenceException>());
}

void natsu::throw_invalid_cast_exception()
{
    throw make_exception(make_object<InvalidCastException>());
}

void natsu::throw_index_out_of_range_exception()
{
    throw make_exception(make_object<IndexOutOfRangeException>());
}

void natsu::throw_overflow_exception()
{
    throw make_exception(make_object<OverflowException>());
}

void natsu::pure_call()
{
    throw std::runtime_error("pure call");
}

native_int ops::ldlen(const O &obj)
{
    check_null_obj_ref(obj);
    return (intptr_t)stack_to<gc_obj_ref<RawSzArrayData>>(obj)->Count;
}

void ops::throw_(gc_obj_ref<Exception> obj)
{
    check_null_obj_ref(obj);
    throw make_exception(obj);
}

#ifndef _MSC_VER
namespace __cxxabiv1
{
std::terminate_handler __terminate_handler = abort;
}
#endif
