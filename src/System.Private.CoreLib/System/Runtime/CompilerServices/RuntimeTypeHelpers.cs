using System;
using System.Collections.Generic;
using System.Text;

namespace System.Runtime.CompilerServices
{
    public static class RuntimeTypeHelpers
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsAssignableFrom<TFrom, TTo>();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern TResult AllocateLike<TTemplate, TResult, T1>();
    }
}
