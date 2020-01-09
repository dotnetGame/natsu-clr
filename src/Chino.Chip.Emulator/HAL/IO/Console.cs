using Chino.Devices.IO;
using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Chino.Chip.Emulator.HAL.IO
{
    public class Console : Chino.Devices.IO.Console, IConsole
    {
        private UIntPtr _readThread;

        public void Install()
        {
            InstallConsoleReadThread();
        }

        private void OnReceive(ConsoleEvent e)
        {
            EventsBuffer.TryWrite(MemoryMarshal.CreateReadOnlySpan(ref e, 1));
            if (e.Type == ConsoleEventType.KeyEvent)
            {
                if (e.Key.KeyDown)
                    ChipControl.Write(e.Key.Char.ToString());
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void InstallConsoleReadThread();
    }
}
