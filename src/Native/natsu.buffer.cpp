#include "System.Private.CoreLib.h"
#include <cstring>

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Runtime::CompilerServices;

void Buffer::_s_BlockCopy(gc_obj_ref<Array> src, int32_t srcOffset, gc_obj_ref<Array> dst, int32_t dstOffset, int32_t count)
{
    check_null_obj_ref(src);
    check_null_obj_ref(dst);
    auto src_arr = src.cast<RawSzArrayData>();
    auto dest_arr = dst.cast<RawSzArrayData>();
    auto element_size_src = src.header().vtable_->ElementSize;
    auto element_size_dest = dst.header().vtable_->ElementSize;
    auto src_begin = reinterpret_cast<uint8_t *>(&src_arr->Data);
    auto src_bytes = src_arr->Count * element_size_src;
    auto dest_begin = reinterpret_cast<uint8_t *>(&dest_arr->Data);
    auto dest_bytes = dest_arr->Count * element_size_dest;

    if (srcOffset < 0 || dstOffset < 0 || count < 0)
        throw_index_out_of_range_exception();
    if ((srcOffset + count) > src_bytes || (dstOffset + count) > dest_bytes)
        throw_exception<ArgumentException>();
    std::memmove(dest_begin, src_begin, count);
}

void Buffer::_s_Memcpy(gc_ptr<uint8_t> dest, gc_ptr<uint8_t> src, int32_t len)
{
    std::memcpy(dest.get(), src.get(), len);
}

void Buffer::_s_Memmove(gc_ptr<uint8_t> dest, gc_ptr<uint8_t> src, uint64_t len)
{
    std::memmove(dest.get(), src.get(), len);
}
