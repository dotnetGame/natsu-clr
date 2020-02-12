using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Chino.Collections;
using Chino.Objects;
using Chino.Threading;

namespace Chino.IO.Devices
{
    public abstract class ConsoleDevice : Device
    {
        public static string? DefaultDevicePath { get; set; }

        public override DeviceType DeviceType => DeviceType.Console;
    }
}
