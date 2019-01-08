using System;
using System.Runtime.CompilerServices;

namespace System
{
    class Haha
    {
        public int[] Value;

        public Haha()
        {
            Value = new[] { 1 };
        }
    }

    public static class Console
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void SayHello(int value);

        public static double GetValue()
        {
            return 100;
        }

        public static void Test()
        {
            var v = new Haha();
            SayHello(v.Value[0]);
        }
    }
}
