#include "Chino.Chip.Emulator.h"
#include "Chino.Kernel.h"
#include <Windows.h>
#include <process.h>

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace Chino_Core::Chino::Threading;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip;
using namespace Chino_Chip_Emulator::Chino::Chip::Emulator::Threading;

namespace
{
uint32_t __stdcall thread_main_thunk(void *arg)
{
    gc_obj_ref<Thread> thread(reinterpret_cast<Thread *>(arg));
    Thread::_s_ThreadMainThunk(thread);
    return Thread::get_ExitCode(thread);
}
}

void System::Threading::Interlocked::_s__MemoryBarrierProcessWide()
{
    FlushProcessWriteBuffers();
}

void System::Threading::Thread::_s_SpinWaitInternal(int32_t iterations)
{
    while (iterations--)
    {
        YieldProcessor();
    }
}

gc_obj_ref<ThreadContext> ArchChipControl::InitializeThreadContext(gc_obj_ref<ArchChipControl> _this, gc_obj_ref<> thread)
{
    auto context = make_object<ArchThreadContext>();
    auto handle = _beginthreadex(nullptr, 0, thread_main_thunk, thread.ptr_, CREATE_SUSPENDED, nullptr);
    assert(handle != -1);
    context->NativeHandle = handle;
    return context;
}
