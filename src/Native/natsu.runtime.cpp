#include "natsu.runtime.h"
#include "System.Private.CorLib.h"
#include <algorithm>

namespace System_Private_CorLib
{
::natsu::gc_obj_ref<::System_Private_CorLib::System::Type> System::Object::GetType()
{
    return ::natsu::null;
}

::System_Private_CorLib::System::Int32 System::Array::GetLength(::System_Private_CorLib::System::Int32 dimension)
{
    assert(dimension == 0);
    return header_.length_;
}
}

namespace natsu
{
gc_obj_ref<::System_Private_CorLib::System::String> load_string(std::u16string_view string)
{
    auto dest = System_Private_CorLib::System::String::_s_FastAllocateString(string.length());
    std::copy(string.begin(), string.end(), &dest->_firstChar);
    (&dest->_firstChar)[string.length()] = 0;
    return dest;
}

std::u16string_view to_string_view(gc_obj_ref<::System_Private_CorLib::System::String> string)
{
    if (!string)
        return {};
    return { reinterpret_cast<const char16_t *>(&string->_firstChar), (size_t)string->_stringLength };
}
} // namespace natsu
