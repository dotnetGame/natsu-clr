#include "System.Private.CoreLib.h"

#if _MSC_VER
#include <intrin.h>
#include <Windows.h>
#endif

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Threading;
using namespace System_Private_CoreLib::System::Runtime::CompilerServices;

int32_t Interlocked::_s_Exchange(gc_ref<int32_t> location1, int32_t value)
{
#if _MSC_VER
    return _InterlockedExchange(reinterpret_cast<volatile long *>(location1.ptr_), value);
#else
    return __sync_lock_test_and_set(reinterpret_cast<volatile int32_t *>(location1.ptr_), value);
#endif
}

int64_t Interlocked::_s_Exchange(gc_ref<int64_t> location1, int64_t value)
{
#if _MSC_VER
    return _InterlockedExchange64(reinterpret_cast<volatile int64_t *>(location1.ptr_), value);
#else
    return __sync_lock_test_and_set(reinterpret_cast<volatile int64_t *>(location1.ptr_), value);
#endif
}

int64_t Interlocked::_s_CompareExchange(gc_ref<int64_t> location1, int64_t value, int64_t comparand)
{
#if _MSC_VER
    return _InterlockedCompareExchange64(reinterpret_cast<volatile int64_t *>(location1.ptr_), value, comparand);
#else
    return __sync_val_compare_and_swap(reinterpret_cast<volatile int64_t *>(location1.ptr_), value, comparand);
#endif
}

gc_obj_ref<Object> Interlocked::_s_CompareExchange(gc_ref<gc_obj_ref<Object>> location1, gc_obj_ref<Object> value, gc_obj_ref<Object> comparand)
{
#if _MSC_VER
    return gc_obj_ref<Object>(reinterpret_cast<Object *>(_InterlockedCompareExchangePointer(
        reinterpret_cast<void *volatile *>(location1.ptr_), reinterpret_cast<void *>(value.ptr_),
        reinterpret_cast<void *>(comparand.ptr_))));
#else
    return gc_obj_ref<Object>(reinterpret_cast<Object *>(__sync_val_compare_and_swap(
        reinterpret_cast<void *volatile *>(location1.ptr_), reinterpret_cast<void *>(value.ptr_),
        reinterpret_cast<void *>(comparand.ptr_))));
#endif
}

int32_t Interlocked::_s_ExchangeAdd(gc_ref<int32_t> location1, int32_t value)
{
#if _MSC_VER
    return _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(location1.ptr_), value);
#else
    return __sync_fetch_and_add(reinterpret_cast<volatile int32_t *>(location1.ptr_), value);
#endif
}

int64_t Interlocked::_s_ExchangeAdd(gc_ref<int64_t> location1, int64_t value)
{
#if _MSC_VER
    return _InterlockedExchangeAdd64(reinterpret_cast<volatile int64_t *>(location1.ptr_), value);
#else
    return __sync_fetch_and_add(reinterpret_cast<volatile int64_t *>(location1.ptr_), value);
#endif
}

void Interlocked::_s__MemoryBarrierProcessWide()
{
#if _MSC_VER
    FlushProcessWriteBuffers();
#endif
}
