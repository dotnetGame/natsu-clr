#include "Chino.Chip.Emulator.h"
#include <Windows.h>

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Chip_Emulator::Chino;

void ChipControl::_s_Initialize()
{
    system(" ");
}

void ChipControl::_s_Write(gc_obj_ref<String> message)
{
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &message->_firstChar, message->_stringLength, nullptr, nullptr);
}
