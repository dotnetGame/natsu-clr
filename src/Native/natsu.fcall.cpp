#include "Chino.Kernel.h"
#include "System.Console.h"
#include <cmath>
#include <random>
#ifdef WIN32
#include <Windows.h>
#endif

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Diagnostics;
using namespace System_Private_CoreLib::System::Runtime;
using namespace System_Private_CoreLib::System::Runtime::CompilerServices;
using namespace System_Private_CoreLib::System::Runtime::InteropServices;
using namespace System_Private_CoreLib::System::Threading;
using namespace Chino_Threading;

gc_obj_ref<Type> Object::GetType(::natsu::gc_obj_ref<Object> _this)
{
    check_null_obj_ref(_this);
    return _this.header().vtable_->runtime_type();
}

gc_obj_ref<MulticastDelegate> MulticastDelegate::_s_CreateDelegateLike(gc_obj_ref<MulticastDelegate> delegate, gc_obj_ref<SZArray_1<Delegate>> invocationList)
{
    auto d_len = invocationList->length();
    if (d_len == 0)
    {
        return nullptr;
    }
    else if (d_len == 1)
    {
        return invocationList->get(0).cast<MulticastDelegate>();
    }
    else
    {
        auto d = gc_alloc(*delegate.header().vtable_, sizeof(MulticastDelegate)).cast<MulticastDelegate>();
        d->_invocationList = invocationList;
        return d;
    }
}

void RuntimeImports::_s_GetRandomBytes(gc_ptr<uint8_t> buffer, int32_t length)
{
    std::mt19937 rbe;
    std::generate_n(buffer.get(), length, std::ref(rbe));
}

void RuntimeImports::_s_RhZeroMemory(gc_ptr<void> b, uint64_t byteLength)
{
    std::memset(b.ptr_, 0, byteLength);
}

int32_t RuntimeHelpers::_s_GetHashCode(::natsu::gc_obj_ref<::System_Private_CoreLib::System::Object> o)
{
    return (int32_t)o.ptr_;
}

void Debugger::_s_BreakInternal()
{
#ifdef WIN32
    DebugBreak();
#else
    assert(false);
#endif
}

bool Debugger::_s_get_IsAttached()
{
#ifdef WIN32
    return IsDebuggerPresent();
#else
    return false;
#endif
}

bool Debugger::_s_LaunchInternal()
{
#ifdef WIN32
    while (IsDebuggerPresent())
        DebugBreak();
    return true;
#else
    return false;
#endif
}

void Debugger::_s_CustomNotification(gc_obj_ref<ICustomDebuggerNotification> data)
{
}

void Environment::_s__Exit(int32_t exitCode)
{
    exit(exitCode);
}

int32_t Environment::_s_get_TickCount()
{
    return (int32_t)_s_get_TickCount64();
}

int64_t Environment::_s_get_TickCount64()
{
    auto scheduler = Chino::Threading::Scheduler::_s_get_Current();
    return (int64_t)Chino::Threading::Scheduler::get_TickCount(scheduler);
}

void Marshal::_s_CopyToNative(gc_obj_ref<Object> source, int32_t startIndex, IntPtr destination, int32_t length)
{
    check_null_obj_ref(source);
    check_null_obj_ref(destination._value);
    auto src = source.cast<RawSzArrayData>();
    auto element_size = source.header().vtable_->ElementSize;

    if ((startIndex + length) > (intptr_t)src->Count)
        throw_index_out_of_range_exception();

    std::memmove(destination._value, &src->Data + (size_t)startIndex * element_size, (size_t)length * element_size);
}

void Marshal::_s_CopyToManaged(IntPtr source, gc_obj_ref<Object> destination, int32_t startIndex, int32_t length)
{
    check_null_obj_ref(source._value);
    check_null_obj_ref(destination);
    auto dest = destination.cast<RawSzArrayData>();
    auto element_size = destination.header().vtable_->ElementSize;

    if ((startIndex + length) > (intptr_t)dest->Count)
        throw_index_out_of_range_exception();

    std::memmove(&dest->Data + (size_t)startIndex * element_size, source._value, (size_t)length * element_size);
}

namespace
{
uint32_t get_current_thread_id()
{
    return Chino::Threading::Scheduler::_s_get_CurrentThreadId();
}
}

void Monitor::_s_Enter(gc_obj_ref<Object> obj)
{
    bool lock_taken = false;
    _s_ReliableEnter(obj, lock_taken);
}

void Monitor::_s_ReliableEnter(gc_obj_ref<Object> obj, gc_ref<bool> lockTaken)
{
    check_null_obj_ref(obj);
    auto thread_id = get_current_thread_id();
    auto &sync_header = obj.header().sync_header_;
    uint32_t expected = 0;

    while (true)
    {
        if (sync_header.lock_taken.compare_exchange_strong(expected, thread_id))
        {
            Volatile::_s_Write(*lockTaken, true);
            break;
        }

        Thread::_s_Sleep(1);
    }
}

void Monitor::_s_Exit(::natsu::gc_obj_ref<Object> obj)
{
    check_null_obj_ref(obj);
    auto &sync_header = obj.header().sync_header_;
    sync_header.lock_taken.store(0);
}

void Monitor::_s_ReliableEnterTimeout(gc_obj_ref<Object> obj, int32_t timeout, gc_ref<bool> lockTaken)
{
    pure_call();
}

bool Monitor::_s_IsEnteredNative(gc_obj_ref<Object> obj)
{
    check_null_obj_ref(obj);
    auto thread_id = get_current_thread_id();
    auto &sync_header = obj.header().sync_header_;
    return sync_header.lock_taken.load() == thread_id;
}

bool Monitor::_s_ObjWait(bool exitContext, int32_t millisecondsTimeout, gc_obj_ref<Object> obj)
{
    pure_call();
}

void Monitor::_s_ObjPulse(gc_obj_ref<Object> obj)
{
    pure_call();
}

void Monitor::_s_ObjPulseAll(gc_obj_ref<Object> obj)
{
    pure_call();
}

int64_t Monitor::_s_GetLockContentionCount()
{
    return 0;
}

void Thread::_s_SleepInternal(int32_t millisecondsTimeout)
{
    Chino::Threading::Scheduler::_s_Delay(TimeSpan::_s_FromMilliseconds(millisecondsTimeout));
}

bool Thread::_s_YieldInternal()
{
    Chino::Threading::Scheduler::_s_Delay(TimeSpan::_s_FromTicks(0));
    return true;
}

int32_t Thread::_s_GetOptimalMaxSpinWaitsPerSpinIterationInternal()
{
    return 1;
}
