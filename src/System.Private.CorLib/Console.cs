using System;
using System.Runtime.CompilerServices;

namespace System
{
    public static class Console
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void SayHello();

        public static void Test()
        {
            SayHello();
        }
    }
}
