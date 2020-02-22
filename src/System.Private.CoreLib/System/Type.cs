using System;
using System.Runtime.CompilerServices;

namespace System
{
    public class Type
    {
        public int this[int i] => 0;

        public static Type GetTypeFromHandle(RuntimeTypeHandle handle)
        {
            return handle._runtimeType;
        }
    }
}
