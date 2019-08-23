#include "natsu.runtime.h"
#include "System.Private.CorLib.h"
#include <algorithm>

void InitializeHeap() noexcept;

namespace natsu
{
gc_obj_ref<::System_Private_CorLib::System::String> load_string(std::u16string_view string)
{
#ifdef WIN32
    InitializeHeap();
#endif
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

void throw_null_ref_exception()
{
    throw make_exception(make_object<System_Private_CorLib::System::NullReferenceException>());
}

void throw_index_out_of_range_exception()
{
    throw make_exception(make_object<System_Private_CorLib::System::IndexOutOfRangeException>());
}
} // namespace natsu
