using Chino.Devices.IO;
using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.CompilerServices;

namespace Chino.Chip.Emulator.HAL.IO
{
    public class Console : IConsole
    {
        public event EventHandler DataAvailable;

        private UIntPtr _readThread;

        public void Install()
        {
            InstallConsoleReadThread();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void InstallConsoleReadThread();
    }
}
