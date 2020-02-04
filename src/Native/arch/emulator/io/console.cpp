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

namespace
{
void ConsoleReadThread(void *arg)
{
    auto this_ = gc_obj_ref<EmulatorConsole>(reinterpret_cast<EmulatorConsole *>(arg));
    wrl::Wrappers::FileHandle cin(GetStdHandle(STD_INPUT_HANDLE));
    THROW_WIN32_IF_NOT(cin.IsValid());

    INPUT_RECORD input_records[16];
    while (true)
    {
        DWORD input_read = 0;
        THROW_WIN32_IF_NOT(ReadConsoleInput(cin.Get(), input_records, std::size(input_records), &input_read));
        for (DWORD i = 0; i < input_read; i++)
        {
            auto &record = input_records[i];

            switch (record.EventType)
            {
            case KEY_EVENT:
                this_->OnReceive(this_, ConsoleEvent::_s_CreateKey(
                    record.Event.KeyEvent.bKeyDown, record.Event.KeyEvent.uChar.UnicodeChar));
                break;
            default:
                break;
            }
        }
    }
}
}

void EmulatorConsole::InstallConsoleReadThread(gc_obj_ref<EmulatorConsole> _this)
{
    auto thread = _beginthread(ConsoleReadThread, 0, _this.ptr_);
    assert(thread);
    THROW_IF_FAILED(SetThreadDescription((HANDLE)thread, L"Driver/Console/Reader"));
}
