using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Chino.IO.Devices;
using Chino.Objects;
using System.Threading;

namespace Chino.Chip.Emulator.IO.Devices
{
    public class EmulatorConsole : ConsoleDevice
    {
        public EmulatorConsole(int capacity)
            : base(capacity)
        {
        }

        protected override void OnInstall()
        {
            InstallConsoleReadThread();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void InstallConsoleReadThread();
    }
}
