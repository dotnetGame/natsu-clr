using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;

namespace Chino.IO
{
    public sealed class DeviceDescription
    {
        internal Accessor<Driver>? _installedDriver;

        public string? DesiredDriver { get; set; }

        public string Type { get; }

        public Dictionary<string, object> Attributes { get; } = new Dictionary<string, object>();

        public DeviceDescription(string type)
        {
            Type = type;
        }
    }
}
