// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Runtime.Versioning;

#if BIT64
using nuint = System.UInt64;
#else
using nuint = System.UInt32;
#endif

namespace System
{
    [Serializable]
    [CLSCompliant(false)]
    public readonly struct UIntPtr : IEquatable<UIntPtr>
    {
        private readonly unsafe void* _value; // Do not rename (binary serialization)

        public static readonly UIntPtr Zero;

        [NonVersionable]
        public unsafe UIntPtr(ulong value)
        {
            _value = (void*)value;
        }

        [NonVersionable]
        public unsafe UIntPtr(void* value)
        {
            _value = value;
        }

        public unsafe override bool Equals(object obj)
        {
            if (obj is UIntPtr)
            {
                return (_value == ((UIntPtr)obj)._value);
            }
            return false;
        }

        unsafe bool IEquatable<UIntPtr>.Equals(UIntPtr other)
        {
            return _value == other._value;
        }

        public unsafe override int GetHashCode()
        {
#if BIT64
            ulong l = (ulong)_value;
            return (unchecked((int)l) ^ (int)(l >> 32));
#else
            return unchecked((int)_value);
#endif
        }

        [NonVersionable]
        public unsafe uint ToUInt32()
        {
#if BIT64
            return checked((uint)_value);
#else
            return (uint)_value;
#endif
        }

        [NonVersionable]
        public unsafe ulong ToUInt64()
        {
            return (ulong)_value;
        }

        [NonVersionable]
        public static explicit operator UIntPtr(uint value)
        {
            return new UIntPtr(value);
        }

        [NonVersionable]
        public static explicit operator UIntPtr(ulong value)
        {
            return new UIntPtr(value);
        }

        [NonVersionable]
        public static unsafe explicit operator UIntPtr(void* value)
        {
            return new UIntPtr(value);
        }

        [NonVersionable]
        public static unsafe explicit operator void* (UIntPtr value)
        {
            return value._value;
        }

        [NonVersionable]
        public static unsafe explicit operator uint(UIntPtr value)
        {
#if BIT64
            return checked((uint)value._value);
#else
            return (uint)value._value;
#endif
        }

        [NonVersionable]
        public static unsafe explicit operator ulong(UIntPtr value)
        {
            return (ulong)value._value;
        }

        [NonVersionable]
        public static unsafe bool operator ==(UIntPtr value1, UIntPtr value2)
        {
            return value1._value == value2._value;
        }

        [NonVersionable]
        public static unsafe bool operator !=(UIntPtr value1, UIntPtr value2)
        {
            return value1._value != value2._value;
        }

        [NonVersionable]
        public static UIntPtr Add(UIntPtr pointer, int offset)
        {
            return pointer + offset;
        }

        [NonVersionable]
        public static unsafe UIntPtr operator +(UIntPtr pointer, int offset)
        {
            return new UIntPtr((nuint)pointer._value + (nuint)offset);
        }

        [NonVersionable]
        public static UIntPtr Subtract(UIntPtr pointer, int offset)
        {
            return pointer - offset;
        }

        [NonVersionable]
        public static unsafe UIntPtr operator -(UIntPtr pointer, int offset)
        {
            return new UIntPtr((nuint)pointer._value - (nuint)offset);
        }

        public static unsafe int Size
        {
            [NonVersionable]
            get
            {
                return sizeof(nuint);
            }
        }

        [NonVersionable]
        public unsafe void* ToPointer()
        {
            return _value;
        }
    }
}
