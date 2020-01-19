#include "Chino.Kernel.h"
#include "win_utils.h"
#include <Windows.h>
#include <process.h>
#include <wrl.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Kernel::Chino;
using namespace Chino_Kernel::Chino::Threading;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip;
using namespace Microsoft::WRL::Wrappers;

namespace
{
std::atomic<bool> g_interrupt_enabled(false);
CRITICAL_SECTION g_interrupt_cs;
Event g_interrupt_enabled_event(CreateEvent(nullptr, TRUE, FALSE, nullptr));
TP_TIMER *g_system_timer;

Semaphore g_interrupt_count(CreateSemaphore(nullptr, 0, 10240, nullptr));
std::atomic<bool> g_system_timer_int(false);

void suspend_thread(ThreadContextArch &context)
{
    auto handle = (HANDLE)context.NativeHandle._value;
    SuspendThread(handle);
    CONTEXT c { CONTEXT_ALL };
    THROW_WIN32_IF_NOT(GetThreadContext(handle, &c));
}

void resume_thread(ThreadContextArch &context)
{
    ResumeThread((HANDLE)context.NativeHandle._value);
}

void notify_interrupt()
{
    THROW_WIN32_IF_NOT(ReleaseSemaphore(g_interrupt_count.Get(), 1, NULL));
}

void system_timer_thunk(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
    g_system_timer_int.store(true, std::memory_order_relaxed);
    notify_interrupt();
}

void interrupt_sender_main(void *arg)
{
    while (true)
    {
        if (g_interrupt_enabled.load(std::memory_order_acquire))
        {
            if (WaitForSingleObject(g_interrupt_count.Get(), INFINITE) == WAIT_ABANDONED)
                break;

            {
                auto leave = natsu::make_finally([&] { LeaveCriticalSection(&g_interrupt_cs); });
                EnterCriticalSection(&g_interrupt_cs);

                ThreadContextArch context;
                // Suspend running thread
                auto running_thread_entry = KernelServices::_s_get_Scheduler()->_runningThread;
                if (running_thread_entry)
                {
                    auto running_thread = running_thread_entry->item->_Thread_k__BackingField;
                    // Wait for suspended
                    suspend_thread(running_thread->Context.Arch);
                    context = running_thread->Context.Arch;
                }

                if (g_system_timer_int.exchange(false))
                {
                    IRQDispatcher::DispatchSystemIRQ(KernelServices::_s_get_IRQDispatcher(),
                        SystemIRQ::from(SystemIRQ::SystemTick), context);
                }
            }
        }
        else
        {
            if (WaitForSingleObject(g_interrupt_enabled_event.Get(), INFINITE) == WAIT_ABANDONED)
                break;
        }
    }
}
}

void ChipControl::_s_Initialize()
{
    system(" ");

    // Init interrupt lock
    THROW_WIN32_IF_NOT(InitializeCriticalSectionEx(&g_interrupt_cs, 300, 0));
    EnterCriticalSection(&g_interrupt_cs);
    // Init interrupt sender thread
    auto intr_thrd = _beginthread(interrupt_sender_main, 0, nullptr);
    assert(intr_thrd);
    assert(g_interrupt_enabled_event.IsValid());
    assert(g_interrupt_count.IsValid());
    THROW_IF_FAILED(SetThreadDescription((HANDLE)intr_thrd, L"Interrupt Sender"));

    auto dev = make_object<Chip::Emulator::HAL::IO::Console>();
    Chip::Emulator::HAL::IO::Console::Install(dev);
}

UIntPtr ChipControl::_s_DisableInterrupt()
{
    if (g_interrupt_enabled.exchange(false))
    {
        EnterCriticalSection(&g_interrupt_cs);
        THROW_WIN32_IF_NOT(ResetEvent(g_interrupt_enabled_event.Get()));
    }

    return 0;
}

UIntPtr ChipControl::_s_EnableInterrupt()
{
    if (!g_interrupt_enabled.exchange(true))
    {
        LeaveCriticalSection(&g_interrupt_cs);
        THROW_WIN32_IF_NOT(SetEvent(g_interrupt_enabled_event.Get()));
    }

    return 1;
}

void ChipControl::_s_RestoreInterrupt(UIntPtr state)
{
    auto old_state = g_interrupt_enabled.exchange(state, std::memory_order_release);
    if (old_state != state)
    {
        if (state)
        {
            EnterCriticalSection(&g_interrupt_cs);
            THROW_WIN32_IF_NOT(SetEvent(g_interrupt_enabled_event.Get()));
        }
        else
        {
            LeaveCriticalSection(&g_interrupt_cs);
            THROW_WIN32_IF_NOT(ResetEvent(g_interrupt_enabled_event.Get()));
        }
    }
}

void ChipControl::_s_StartSchedule(gc_ref<ThreadContextArch> context)
{
    resume_thread(*context);
    _s_EnableInterrupt();
    ExitThread(0);
}

void ChipControl::_s_SetupSystemTimer(TimeSpan timeSlice)
{
    FILETIME due_time = ticks_to_filetime(TimeSpan::get_Ticks(timeSlice));
    g_system_timer = CreateThreadpoolTimer(system_timer_thunk, nullptr, nullptr);
    THROW_WIN32_IF_NOT(g_system_timer);
    SetThreadpoolTimer(g_system_timer, &due_time, (DWORD)TimeSpan::get_TotalMilliseconds(timeSlice), 0);
}

void ChipControl::_s_RestoreContext(gc_ref<ThreadContextArch> context)
{
    // Run selected thread
    resume_thread(*context);
}
