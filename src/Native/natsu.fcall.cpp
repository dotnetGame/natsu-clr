#include "Chino.Kernel.h"
#include <cmath>
#include <cstring>
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
using namespace System_Private_CoreLib::System::Threading;
using namespace Chino_Core;

int32_t Array::GetLength(gc_obj_ref<Array> _this, int32_t dimension)
{
    if (dimension != 0)
        throw_exception<IndexOutOfRangeException>();
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

void Buffer::_s_Memcpy(gc_ptr<uint8_t> dest, gc_ptr<uint8_t> src, int32_t len)
{
    std::memcpy(dest.get(), src.get(), len);
}

void Debug::_s_WriteCore(gc_obj_ref<String> message)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
}

void Debug::_s_WriteLineCore(gc_obj_ref<String> message)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(::System_Private_CoreLib::System::Environment::_s_get_NewLine());
}

void Debug::_s_FailCore(gc_obj_ref<String> message, gc_obj_ref<String> detailMessage)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
#ifdef WIN32
    DebugBreak();
#endif
}

void Buffer::_s_Memmove(gc_ptr<uint8_t> dest, gc_ptr<uint8_t> src, uint64_t len)
{
    std::memmove(dest.get(), src.get(), len);
}

gc_obj_ref<Type> Object::GetType(::natsu::gc_obj_ref<Object> _this)
{
    check_null_obj_ref(_this);
    pure_call();
}

char16_t String::get_Chars(gc_obj_ref<String> _this, int32_t index)
{
    if ((uint32_t)index >= (uint32_t)_this->_stringLength)
        throw_exception<IndexOutOfRangeException>();
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

double Math::_s_Abs(double value)
{
    return fabs(value);
}

float Math::_s_Abs(float value)
{
    return fabsf(value);
}

double Math::_s_Acos(double value)
{
    return acos(value);
}

double Math::_s_Acosh(double value)
{
    return acosh(value);
}

double Math::_s_Asin(double value)
{
    return asin(value);
}

double Math::_s_Asinh(double value)
{
    return asinh(value);
}

double Math::_s_Atan(double value)
{
    return atan(value);
}

double Math::_s_Atan2(double y, double x)
{
    return atan2(y, x);
}

double Math::_s_Atanh(double value)
{
    return atanh(value);
}

double Math::_s_Cbrt(double value)
{
    return cbrt(value);
}

double Math::_s_Ceiling(double value)
{
    return ceil(value);
}

double Math::_s_Cos(double value)
{
    return cos(value);
}

double Math::_s_Cosh(double value)
{
    return cosh(value);
}

double Math::_s_Exp(double value)
{
    return exp(value);
}

double Math::_s_Floor(double value)
{
    return floor(value);
}

double Math::_s_Log(double value)
{
    return log(value);
}

double Math::_s_Log10(double value)
{
    return log10(value);
}

double Math::_s_Pow(double x, double y)
{
    return pow(x, y);
}

double Math::_s_Sin(double value)
{
    return sin(value);
}

double Math::_s_Sinh(double value)
{
    return sinh(value);
}

double Math::_s_Sqrt(double value)
{
    return sqrt(value);
}

double Math::_s_Tan(double value)
{
    return tan(value);
}

double Math::_s_Tanh(double value)
{
    return tanh(value);
}

double Math::_s_FMod(double x, double y)
{
    return fmod(x, y);
}

double Math::_s_ModF(double x, gc_ptr<double> y)
{
    return fmod(x, *y);
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
    auto scheduler = Chino::KernelServices::_s_get_Scheduler();
    return (int64_t)Chino::Threading::Scheduler::get_TickCount(scheduler);
}

namespace
{
void *get_current_thread_id()
{
    auto thread = Chino::KernelServices::_s_get_CurrentThread();
    return thread.ptr_;
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
    void *expected = nullptr;

    while (true)
    {
        if (sync_header.lock_taken.compare_exchange_strong(expected, thread_id))
        {
            Volatile::_s_Write(*lockTaken, true);
            break;
        }

        Thread::_s_SleepInternal(1);
    }
}

void Monitor::_s_Exit(::natsu::gc_obj_ref<Object> obj)
{
    check_null_obj_ref(obj);
    auto thread_id = get_current_thread_id();
    auto &sync_header = obj.header().sync_header_;
    void *expected = thread_id;
    sync_header.lock_taken.compare_exchange_strong(expected, nullptr);
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
    auto scheduler = Chino::KernelServices::_s_get_Scheduler();
    Chino::Threading::Scheduler::DelayCurrentThread(scheduler, TimeSpan::_s_FromMilliseconds(millisecondsTimeout));
}

bool Thread::_s_YieldInternal()
{
    auto scheduler = Chino::KernelServices::_s_get_Scheduler();
    Chino::Threading::Scheduler::DelayCurrentThread(scheduler, TimeSpan::_s_FromTicks(0));
    return true;
}

int32_t Thread::_s_GetOptimalMaxSpinWaitsPerSpinIterationInternal()
{
    return 1;
}
