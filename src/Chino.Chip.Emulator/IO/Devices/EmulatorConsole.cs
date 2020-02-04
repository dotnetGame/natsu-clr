using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Chino.IO.Devices;

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

        private void OnReceive(ConsoleEvent e)
        {
            EventsBuffer.TryWrite(MemoryMarshal.CreateReadOnlySpan(ref e, 1));
            if (e.Type == ConsoleEventType.KeyEvent)
            {
                if (e.Key.KeyDown)
                    Debug.Write(e.Key.Char.ToString());
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void InstallConsoleReadThread();
    }
}
