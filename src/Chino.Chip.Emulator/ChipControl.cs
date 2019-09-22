using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;

namespace Chino
{
    public static class ChipControl
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Initialize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Write(string message);
    }
}
