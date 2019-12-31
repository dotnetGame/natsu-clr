using System;
using System.Runtime.CompilerServices;

namespace Chino
{
    public static class IOVolatile
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ref T As<T>(uint address) where T : unmanaged;

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern T Read<T>(ref T address) where T : unmanaged;

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Write<T>(ref T address, T value) where T : unmanaged;
    }
}
