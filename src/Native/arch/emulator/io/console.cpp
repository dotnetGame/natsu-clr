#include "Chino.Chip.Emulator.h"
#include <Windows.h>
#include <process.h>
#include <wrl.h>
// clang-format off
#include "../win_utils.h"
// clang-format on

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip::Emulator::IO::Devices;
using namespace Chino_IO::Chino::IO::Devices;
namespace wrl = Microsoft::WRL;

void EmulatorConsole::OpenStdHandles(gc_obj_ref<EmulatorConsole> _this)
{
    _this->_stdIn = (uintptr_t)GetStdHandle(STD_INPUT_HANDLE);
    _this->_stdOut = (uintptr_t)GetStdHandle(STD_OUTPUT_HANDLE);

    THROW_WIN32_IF_NOT(SetConsoleCP(CP_UTF8));
    THROW_WIN32_IF_NOT(SetConsoleOutputCP(CP_UTF8));
    THROW_WIN32_IF_NOT(SetConsoleMode(_this->_stdIn._value,
        ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT));
}

int32_t EmulatorConsole::Read(gc_obj_ref<EmulatorConsole> _this, Span_1<uint8_t> buffer)
{
    DWORD read = 0;
    THROW_WIN32_IF_NOT(ReadFile(_this->_stdIn._value, buffer._pointer._value._value, buffer._length, &read, nullptr));
    return read;
}

void EmulatorConsole::Write(gc_obj_ref<EmulatorConsole> _this, ReadOnlySpan_1<uint8_t> buffer)
{
    DWORD written = 0;
    THROW_WIN32_IF_NOT(WriteFile(_this->_stdOut._value, buffer._pointer._value._value, buffer._length, &written, nullptr));
    assert(written == buffer._length);
}
