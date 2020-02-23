#include "System.Private.CoreLib.h"
#include <cstring>

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;

char16_t String::get_Chars(gc_obj_ref<String> _this, int32_t index)
{
    if ((uint32_t)index >= (uint32_t)_this->_stringLength)
        throw_index_out_of_range_exception();
    return (&_this->_firstChar)[index];
}

int32_t String::get_Length(gc_obj_ref<String> _this)
{
    return _this->_stringLength;
}

gc_obj_ref<String> String::_s_FastAllocateString(int32_t length)
{
    auto size = sizeof(String) + length * sizeof(Char);
    auto obj = natsu::gc_new<String>(size);
    obj->_stringLength = length;
    (&obj->_firstChar)[length] = 0;
    return obj;
}

int32_t String::_s_wcslen(gc_ptr<char16_t> ptr)
{
    std::u16string_view sv(ptr.get());
    return (int32_t)sv.length();
}
