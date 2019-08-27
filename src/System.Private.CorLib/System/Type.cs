using System;
using System.Runtime.CompilerServices;

namespace System
{
    public class Type
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern Type GetTypeFromHandle(RuntimeTypeHandle handle);
    }
}
