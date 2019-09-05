// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace System
{
    [Serializable]
    [StructLayout(LayoutKind.Sequential)]
    [TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public readonly struct Byte : IComparable, IComparable<byte>, IEquatable<byte>
    {
        private readonly byte m_value; // Do not rename (binary serialization)

        // The maximum value that a Byte may represent: 255.
        public const byte MaxValue = (byte)0xFF;

        // The minimum value that a Byte may represent: 0.
        public const byte MinValue = 0;


        // Compares this object to another object, returning an integer that
        // indicates the relationship. 
        // Returns a value less than zero if this  object
        // null is considered to be less than any instance.
        // If object is not of type byte, this method throws an ArgumentException.
        // 
        public int CompareTo(object value)
        {
            if (value == null)
            {
                return 1;
            }
            if (!(value is byte))
            {
                throw new ArgumentException(SR.Arg_MustBeByte);
            }

            return m_value - (((byte)value).m_value);
        }

        public int CompareTo(byte value)
        {
            return m_value - value;
        }

        // Determines whether two Byte objects are equal.
        public override bool Equals(object obj)
        {
            if (!(obj is byte))
            {
                return false;
            }
            return m_value == ((byte)obj).m_value;
        }

        [NonVersionable]
        public bool Equals(byte obj)
        {
            return m_value == obj;
        }

        // Gets a hash code for this instance.
        public override int GetHashCode()
        {
            return m_value;
        }
    }
}
