#include "Chino.Chip.Emulator.h"
#include <Windows.h>
#include <process.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip::Emulator::HAL::IO;

namespace
{
void ConsoleReadThread(void *arg)
{
}
}

void Console::InstallConsoleReadThread_(::natsu::gc_obj_ref<::Chino_Chip_Emulator::Chino::Chip::Emulator::HAL::IO::Console> _this)
{
    auto thread = _beginthread(ConsoleReadThread, 0, _this.ptr_);
    assert(thread);
}
