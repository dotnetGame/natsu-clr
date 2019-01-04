// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System
{
    public struct Byte
    {
        private byte m_value; // Do not rename (binary serialization)

        public const byte MaxValue = (byte)0xFF;
        public const byte MinValue = 0;
    }
}
