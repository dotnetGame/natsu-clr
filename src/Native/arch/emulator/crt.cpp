#include "Chino.Chip.Emulator.h"
#include <Windows.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Chip_Emulator::Chino::Chip;

void ArchChipControl::Write(gc_obj_ref<ArchChipControl> _this, gc_obj_ref<String> message)
{
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
}
