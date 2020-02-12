using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chip.Emulator.IO.Devices;
using Chino.IO;

namespace Chino.Chip.Emulator.IO.Drivers
{
    public sealed class ConsoleDriver : Driver
    {
        protected override void InstallDevice(DeviceDescription deviceDescription)
        {
            IOManager.InstallDevice(new EmulatorConsole());
        }

        protected override bool IsCompatible(DeviceDescription deviceDescription)
        {
            return deviceDescription.Type == "emulator, console";
        }
    }
}
