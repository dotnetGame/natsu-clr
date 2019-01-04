// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System
{
    public struct UInt16
    {
        private ushort m_value; // Do not rename (binary serialization)

        public const ushort MaxValue = (ushort)0xFFFF;
        public const ushort MinValue = 0;
    }
}
