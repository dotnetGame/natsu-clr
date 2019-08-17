#include "natsu.runtime.h"
#include "System.Private.CorLib.h"
#include <algorithm>

namespace System_Private_CorLib
{
::natsu::gc_ptr<::System_Private_CorLib::System::AttributeUsageAttribute> System::AttributeUsageAttribute::Default;

::natsu::gc_ptr<::System_Private_CorLib::System::Type> System::Object::GetType()
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
gc_ptr<::System_Private_CorLib::System::String> load_string(std::u16string_view string)
{
    auto size = sizeof(::System_Private_CorLib::System::String) + std::max(0, (int32_t)string.length() - 1);
    auto obj = gc_alloc(size);
    auto ptr = reinterpret_cast<::System_Private_CorLib::System::String *>(obj);
    ptr->_stringLength = string.length();
    std::copy(string.begin(), string.end(), &ptr->_firstChar);
    return gc_ptr<::System_Private_CorLib::System::String>(ptr);
}
} // namespace  natsu
