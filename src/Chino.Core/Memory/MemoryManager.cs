using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;

namespace Chino.Memory
{
    public class MemoryManager
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetUsedMemorySize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetFreeMemorySize();
    }
}
