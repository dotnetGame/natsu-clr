using System;
using System.Runtime.CompilerServices;

namespace System
{
    public static class Console
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void SayHello(float value);

        public static double GetValue()
        {
            return 100;
        }

        public static void Test()
        {
            SayHello((float)GetValue());
        }
    }
}
