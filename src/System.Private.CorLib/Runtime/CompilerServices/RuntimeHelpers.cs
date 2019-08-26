using System;
using System.Collections.Generic;
using Internal.Runtime.CompilerServices;

namespace System.Runtime.CompilerServices
{
    public static partial class RuntimeHelpers
    {
        [Intrinsic]
        internal static ref byte GetRawSzArrayData(this Array array) =>
            ref Unsafe.As<RawSzArrayData>(array).Data;
    }
}
