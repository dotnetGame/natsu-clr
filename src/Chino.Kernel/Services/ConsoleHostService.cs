using Chino.IO;
using Chino.IO.Devices;
using Chino.Objects;
using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Services
{
    public sealed class ConsoleHostService
    {
        public static ConsoleHostService Current { get; } = new ConsoleHostService();

        public Accessor<Device> Stdin { get; }

        public Accessor<Device> Stdout { get; }

        public Accessor<Device> Stderr { get; }

        public ConsoleHostService()
        {
            var defaultConsole = ConsoleDevice.DefaultDevicePath;

            Stdin = ObjectManager.OpenObject<Device>(AccessMask.GenericRead, new ObjectAttributes { Name = defaultConsole });
            Stdout = ObjectManager.OpenObject<Device>(AccessMask.GenericWrite, new ObjectAttributes { Name = defaultConsole });
            Stderr = ObjectManager.OpenObject<Device>(AccessMask.GenericWrite, new ObjectAttributes { Name = defaultConsole });
        }
    }
}
