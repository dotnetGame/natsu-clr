#include "Chino.Chip.Emulator.h"
#include "Chino.Kernel.h"
#include <Windows.h>
#include <process.h>

using namespace natsu;
using namespace System_Threading::System::Threading;
using namespace Chino_Core::Chino::Threading;
using namespace Chino_Kernel::Chino::Threading;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip;

namespace
{
uint32_t __stdcall thread_main_thunk(void *arg)
{
    gc_obj_ref<Thread> thread(reinterpret_cast<Thread *>(arg));
    return Thread::_s_ThreadMainThunk(thread);
}
}

void Interlocked::_s__MemoryBarrierProcessWide()
{
    FlushProcessWriteBuffers();
}

void ChipControl::_s_InitializeThreadContext(gc_ref<ThreadContextArch> context, gc_obj_ref<IThread> thread)
{
    auto handle = _beginthreadex(nullptr, 0, thread_main_thunk, thread.ptr_, CREATE_SUSPENDED, nullptr);
    assert(handle != -1);
    context->NativeHandle = handle;
}
