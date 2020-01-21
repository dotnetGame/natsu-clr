#include "Chino.Chip.Emulator.h"
#include <Windows.h>
#include <process.h>
#include <wrl.h>
// clang-format off
#include "../../win_utils.h"
// clang-format on

using namespace natsu;
using namespace System_Runtime::System;
using namespace Chino_Chip_Emulator::Chino;
using namespace Chino_Chip_Emulator::Chino::Chip::Emulator::HAL::IO;
namespace wrl = Microsoft::WRL;
namespace cio = Chino_Core::Chino::Devices::IO;

namespace
{
void ConsoleReadThread(void *arg)
{
    auto this_ = gc_obj_ref<Console>(reinterpret_cast<Console *>(arg));
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
            cio::ConsoleEvent cevent;

            switch (record.EventType)
            {
            case KEY_EVENT:
                cevent._Type_k__BackingField = cio::ConsoleEventType::KeyEvent;
                cevent._Key_k__BackingField._KeyDown_k__BackingField = record.Event.KeyEvent.bKeyDown;
                cevent._Key_k__BackingField._Char_k__BackingField = record.Event.KeyEvent.uChar.UnicodeChar;
                this_->OnReceive(this_, cevent);
                break;
            default:
                break;
            }
        }
    }
}
}

void Console::InstallConsoleReadThread(::natsu::gc_obj_ref<::Chino_Chip_Emulator::Chino::Chip::Emulator::HAL::IO::Console> _this)
{
    auto thread = _beginthread(ConsoleReadThread, 0, _this.ptr_);
    assert(thread);
    THROW_WIN32_IF_NOT(SetThreadDescription((HANDLE)thread, L"Driver/Console/Reader"));
}
