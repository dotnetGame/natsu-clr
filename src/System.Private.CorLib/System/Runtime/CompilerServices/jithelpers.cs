using System;
using System.Collections.Generic;

namespace System.Runtime.CompilerServices
{
    internal class RawSzArrayData
    {
        public IntPtr Count; // Array._numComponents padded to IntPtr
        public byte Data;
    }

    internal class RawStringData
    {
        public int Length;
        public char Data;
    }

    // Helper class to assist with unsafe pinning of arbitrary objects.
    // It's used by VM code.
    internal class RawData
    {
        public byte Data;
    }
}
