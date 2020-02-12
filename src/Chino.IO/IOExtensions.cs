using System;
using System.Collections.Generic;
using System.Text;
using Chino.IO;
using Chino.Objects;

namespace Chino
{
    public static class IOExtensions
    {
        public static int Read(this IAccessor<Device> device, Span<byte> buffer)
        {
            return device.Object.Read(buffer);
        }

        public static void Write(this IAccessor<Device> device, ReadOnlySpan<byte> buffer)
        {
            device.Object.Write(buffer);
        }
    }
}
