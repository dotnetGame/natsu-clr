#include "Chino.Kernel.h"
#include "win_utils.h"
#include <Windows.h>
#include <wrl.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Kernel::Chino::Threading;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip;
using namespace Microsoft::WRL::Wrappers;

namespace
{
bool g_interrupt_enabled = false;
Event g_system_timer;

void resume_thread(ThreadContextArch &context)
{
    ResumeThread((HANDLE)context.NativeHandle._value);
}

void system_timer_thunk(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
}
}

UIntPtr ChipControl::_s_DisableInterrupt()
{
    g_interrupt_enabled = false;
    return g_interrupt_enabled;
}

UIntPtr ChipControl::_s_EnableInterrupt()
{
    g_interrupt_enabled = true;
    return g_interrupt_enabled;
}

void ChipControl::_s_RestoreInterrupt(UIntPtr state)
{
    g_interrupt_enabled = state;
}

void ChipControl::_s_StartSchedule(gc_ref<ThreadContextArch> context)
{
    resume_thread(*context);
    _s_EnableInterrupt();
    //ExitThread(0);
}

void ChipControl::_s_SetupSystemTimer(TimeSpan timeSlice)
{
    g_system_timer.Attach(CreateWaitableTimer(nullptr, FALSE, nullptr));
    THROW_WIN32_IF_NOT(g_system_timer.IsValid());

    LARGE_INTEGER due_time;
    due_time.QuadPart = TimeSpan::get_Ticks(timeSlice);
    THROW_WIN32_IF_NOT(SetWaitableTimer(g_system_timer.Get(), &due_time, due_time.QuadPart, system_timer_thunk, nullptr, FALSE));
}
