using Chino.Devices.IO;
using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Chip.Emulator.HAL.IO
{
    public class Console : IConsole
    {
        public event EventHandler DataAvailable;
    }
}
