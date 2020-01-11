using System;
using System.Runtime.CompilerServices;

namespace System
{
    public class Type
    {
        public static Type GetTypeFromHandle(RuntimeTypeHandle handle)
        {
            return handle._runtimeType;
        }
    }
}
