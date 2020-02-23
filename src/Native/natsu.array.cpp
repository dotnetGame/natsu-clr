#include "System.Private.CoreLib.h"

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Runtime::CompilerServices;

int32_t Array::GetLength(gc_obj_ref<Array> _this, int32_t dimension)
{
    if (dimension != 0)
        throw_index_out_of_range_exception();
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

int32_t Array::get_Rank(gc_obj_ref<Array> _this)
{
    return 1;
}

int32_t Array::get_Length(gc_obj_ref<Array> _this)
{
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

int64_t Array::get_LongLength(gc_obj_ref<Array> _this)
{
    return (intptr_t)_this.cast<RawSzArrayData>()->Count;
}

int32_t Array::GetUpperBound(gc_obj_ref<Array> _this, int32_t dimension)
{
    return GetLength(_this, dimension);
}

int32_t Array::GetLowerBound(gc_obj_ref<Array> _this, int32_t dimension)
{
    if (dimension != 0)
        throw_exception<IndexOutOfRangeException>();
    return 0;
}

void Array::_s_Copy(gc_obj_ref<Array> sourceArray, int32_t sourceIndex, gc_obj_ref<Array> destinationArray, int32_t destinationIndex, int32_t length, bool reliable)
{
    check_null_obj_ref(sourceArray);
    check_null_obj_ref(destinationArray);
    auto src = sourceArray.cast<RawSzArrayData>();
    auto dest = sourceArray.cast<RawSzArrayData>();
    auto element_size = sourceArray.header().vtable_->ElementSize;

    if ((sourceIndex + length) > (intptr_t)src->Count || (destinationIndex + length) > (intptr_t)dest->Count)
        throw_index_out_of_range_exception();

    if (sourceArray.header().vtable_ != destinationArray.header().vtable_)
        throw_exception<ArrayTypeMismatchException>();
    std::memmove(&dest->Data + (size_t)destinationIndex * element_size, &src->Data + (size_t)sourceIndex * element_size, (size_t)length * element_size);
}

gc_ref<uint8_t> Array::_s_GetRawArrayGeometry(gc_obj_ref<Array> array, gc_ref<uint32_t> numComponents, gc_ref<uint32_t> elementSize, gc_ref<int32_t> lowerBound, ::natsu::gc_ref<bool> containsGCPointers)
{
    *numComponents = (uint32_t)Array::GetLength(array, 0);
    *elementSize = array.header().vtable_->ElementSize;
    *lowerBound = Array::GetLowerBound(array, 0);
    *containsGCPointers = true;
    return array.cast<RawSzArrayData>()->Data;
}

bool Array::_s_TrySZReverse(gc_obj_ref<Array> array, int32_t index, int32_t count)
{
    check_null_obj_ref(array);
    auto src = array.cast<RawSzArrayData>();
    auto element_size = array.header().vtable_->ElementSize;
    auto data = reinterpret_cast<uint8_t *>(&src->Data);

    if ((index + count) > (intptr_t)src->Count)
        return false;

    size_t i = index, j = index + count - 1;
    void *tmp = alloca(element_size);
    while (i < j)
    {
        std::memcpy(tmp, data + i * element_size, element_size);
        std::memcpy(data + j * element_size, data + i * element_size, element_size);
        std::memcpy(data + j * element_size, tmp, element_size);

        i++;
        j--;
    }

    return true;
}
