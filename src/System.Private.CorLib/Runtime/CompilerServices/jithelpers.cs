using System;
using System.Collections.Generic;

namespace System.Runtime.CompilerServices
{
    internal class RawSzArrayData
    {
        public IntPtr Count; // Array._numComponents padded to IntPtr
        public byte Data;
    }
}
