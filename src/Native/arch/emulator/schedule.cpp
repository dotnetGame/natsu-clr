#include "Chino.Chip.Emulator.h"
#include "Chino.Kernel.h"
#include "win_utils.h"
#include <Windows.h>
#include <process.h>
#include <wrl.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace System_Threading::System::Threading;
using namespace Chino_Core::Chino;
using namespace Chino_Core::Chino::Threading;
using namespace Chino_Threading::Chino::Threading;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip;
using namespace Chino_Chip_Emulator::Chino::Chip::Emulator::Threading;
using namespace Microsoft::WRL::Wrappers;

namespace
{
std::atomic<bool> g_interrupt_enabled(false);
CRITICAL_SECTION g_interrupt_cs;
HANDLE g_system_timer;
TimeSpan g_time_slice;

Semaphore g_interrupt_count(CreateSemaphore(nullptr, 0, 10240, nullptr));
Semaphore g_interrupt_commit_count(CreateSemaphore(nullptr, 0, 10240, nullptr));
std::atomic<bool> g_system_timer_int(false);
std::atomic<bool> g_core_notifi_int(false);
std::atomic<uint32_t> g_interrupt_num(0);

void suspend_thread(ArchThreadContext &context)
{
    //printf("S (%d)\n", (int)GetThreadId(context.NativeHandle._value));
    auto handle = (HANDLE)context.NativeHandle._value;
    THROW_WIN32_IF_NOT(SuspendThread(handle) != DWORD(-1));
    CONTEXT c { CONTEXT_ALL };
    THROW_WIN32_IF_NOT(GetThreadContext(handle, &c));
}

void resume_thread(ArchThreadContext &context)
{
    //printf("R (%d)\n", (int)GetThreadId(context.NativeHandle._value));
    THROW_WIN32_IF_NOT(ResumeThread((HANDLE)context.NativeHandle._value) != (DWORD)(-1));
}

void notify_interrupt()
{
    g_interrupt_num.fetch_add(1);
    THROW_WIN32_IF_NOT(ReleaseSemaphore(g_interrupt_count.Get(), 1, NULL));
}

void wait_interrupt_clear()
{
    while (g_interrupt_num.load())
    {
        if (WaitForSingleObject(g_interrupt_commit_count.Get(), INFINITE) == WAIT_ABANDONED)
            break;
        g_interrupt_num.fetch_sub(1);
    }
}

void system_timer_main(void *arg)
{
    g_system_timer = CreateWaitableTimer(nullptr, FALSE, nullptr);
    THROW_WIN32_IF_NOT(g_system_timer != INVALID_HANDLE_VALUE);

    LARGE_INTEGER due_time;
    due_time.QuadPart = TimeSpan::get_Ticks(g_time_slice);
    THROW_WIN32_IF_NOT(SetWaitableTimer(g_system_timer, &due_time, (DWORD)TimeSpan::get_TotalMilliseconds(g_time_slice), nullptr, nullptr, FALSE));

    while (true)
    {
        if (WaitForSingleObject(g_system_timer, INFINITE) == WAIT_ABANDONED)
            break;

        g_system_timer_int.store(true, std::memory_order_relaxed);
        notify_interrupt();
    }
}

void interrupt_sender_main(void *arg)
{
    while (true)
    {
        if (WaitForSingleObject(g_interrupt_count.Get(), INFINITE) == WAIT_ABANDONED)
            break;

        {
            auto leave = natsu::make_finally([&] { LeaveCriticalSection(&g_interrupt_cs); });
            EnterCriticalSection(&g_interrupt_cs);

            gc_obj_ref<ArchThreadContext> context;
            // Suspend running thread
            auto running_thread_entry = Scheduler::get_RunningThread(Scheduler::_s_get_Current());
            if (running_thread_entry)
            {
                auto running_thread = ThreadScheduleEntry::get_Thread(running_thread_entry->item);
                // Wait for suspended
                context = Thread::get_Context(running_thread);
                suspend_thread(*context);
            }

            THROW_WIN32_IF_NOT(ReleaseSemaphore(g_interrupt_commit_count.Get(), 1, nullptr));

            if (g_system_timer_int.exchange(false))
            {
                IRQDispatcher::_s_DispatchSystemIRQ(SystemIRQ::value_of(SystemIRQ::SystemTick), context);
            }
            else if (g_core_notifi_int.exchange(false))
            {
                IRQDispatcher::_s_DispatchSystemIRQ(SystemIRQ::value_of(SystemIRQ::CoreNotification), context);
            }
        }
    }
}
}

void ArchChipControl::Initialize(gc_obj_ref<ArchChipControl> _this)
{
    system(" ");

    // Init interrupt lock
    THROW_WIN32_IF_NOT(InitializeCriticalSectionEx(&g_interrupt_cs, 300, 0));
    EnterCriticalSection(&g_interrupt_cs);
    // Init interrupt sender thread
    auto intr_thrd = _beginthread(interrupt_sender_main, 0, nullptr);
    assert(intr_thrd);
    assert(g_interrupt_count.IsValid());
    assert(g_interrupt_commit_count.IsValid());
    THROW_IF_FAILED(::SetThreadDescription((HANDLE)intr_thrd, L"Interrupt Sender"));
}

UIntPtr ArchChipControl::DisableInterrupt(gc_obj_ref<ArchChipControl> _this)
{
    auto old = g_interrupt_enabled.exchange(false);
    if (old)
    {
        EnterCriticalSection(&g_interrupt_cs);
    }

    return old;
}

UIntPtr ArchChipControl::EnableInterrupt(gc_obj_ref<ArchChipControl> _this)
{
    auto old = g_interrupt_enabled.exchange(true);
    if (!old)
    {
        LeaveCriticalSection(&g_interrupt_cs);
        wait_interrupt_clear();
    }

    return old;
}

void ArchChipControl::RestoreInterrupt(gc_obj_ref<ArchChipControl> _this, UIntPtr state)
{
    auto old_state = g_interrupt_enabled.exchange(state, std::memory_order_release);
    if (old_state != state)
    {
        if (state)
        {
            LeaveCriticalSection(&g_interrupt_cs);
            wait_interrupt_clear();
        }
        else
        {
            EnterCriticalSection(&g_interrupt_cs);
        }
    }
}

void ArchChipControl::StartSchedule(gc_obj_ref<ArchChipControl> _this, gc_obj_ref<ThreadContext> context)
{
    resume_thread(*context.cast<ArchThreadContext>());
    EnableInterrupt(_this);
    ExitThread(0);
}

void ArchChipControl::SetupSystemTimer(gc_obj_ref<ArchChipControl> _this, TimeSpan timeSlice)
{
    g_time_slice = timeSlice;
    auto systimer_thrd = _beginthread(system_timer_main, 0, nullptr);
    assert(systimer_thrd);
    THROW_IF_FAILED(::SetThreadDescription((HANDLE)systimer_thrd, L"System Timer"));
}

void ArchChipControl::RestoreContext(gc_obj_ref<ArchChipControl> _this, gc_obj_ref<ThreadContext> context)
{
    // Run selected thread
    resume_thread(*context.cast<ArchThreadContext>());
}

void ArchChipControl::RaiseCoreNotification(gc_obj_ref<ArchChipControl> _this)
{
    g_core_notifi_int.store(true, std::memory_order_relaxed);
    notify_interrupt();
}

void ArchChipControl::SetThreadDescription(gc_obj_ref<ArchChipControl> _this, gc_obj_ref<ThreadContext> context, gc_obj_ref<String> value)
{
    THROW_IF_FAILED(::SetThreadDescription(context.cast<ArchThreadContext>()->NativeHandle._value, reinterpret_cast<PCWSTR>(&value->_firstChar)));
}
