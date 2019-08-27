// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Runtime.Versioning;

#if BIT64
using nint = System.Int64;
#else
using nint = System.Int32;
#endif

namespace System
{
    [Serializable]
    public readonly struct IntPtr : IEquatable<IntPtr>
    {
        // WARNING: We allow diagnostic tools to directly inspect this member (_value). 
        // See https://github.com/dotnet/corert/blob/master/Documentation/design-docs/diagnostics/diagnostics-tools-contract.md for more details. 
        // Please do not change the type, the name, or the semantic usage of this member without understanding the implication for tools. 
        // Get in touch with the diagnostics team if you have questions.
        private readonly unsafe void* _value; // Do not rename (binary serialization)

        public static readonly IntPtr Zero;

        [NonVersionable]
        public unsafe IntPtr(long value)
        {
            _value = (void*)value;
        }

        [CLSCompliant(false)]
        [NonVersionable]
        public unsafe IntPtr(void* value)
        {
            _value = value;
        }

        public unsafe override bool Equals(object obj)
        {
            if (obj is IntPtr)
            {
                return (_value == ((IntPtr)obj)._value);
            }
            return false;
        }

        unsafe bool IEquatable<IntPtr>.Equals(IntPtr other)
        {
            return _value == other._value;
        }

        public unsafe override int GetHashCode()
        {
#if BIT64
            long l = (long)_value;
            return (unchecked((int)l) ^ (int)(l >> 32));
#else
            return unchecked((int)_value);
#endif
        }

        [NonVersionable]
        public unsafe int ToInt32()
        {
#if BIT64
            long l = (long)_value;
            return checked((int)l);
#else
            return (int)_value;
#endif
        }

        [NonVersionable]
        public unsafe long ToInt64()
        {
            return (nint)_value;
        }

        [NonVersionable]
        public static unsafe explicit operator IntPtr(int value)
        {
            return new IntPtr(value);
        }

        [NonVersionable]
        public static unsafe explicit operator IntPtr(long value)
        {
            return new IntPtr(value);
        }

        [CLSCompliant(false)]
        [NonVersionable]
        public static unsafe explicit operator IntPtr(void* value)
        {
            return new IntPtr(value);
        }

        [CLSCompliant(false)]
        [NonVersionable]
        public static unsafe explicit operator void* (IntPtr value)
        {
            return value._value;
        }

        [NonVersionable]
        public static unsafe explicit operator int(IntPtr value)
        {
#if BIT64
            long l = (long)value._value;
            return checked((int)l);
#else
            return (int)value._value;
#endif
        }

        [NonVersionable]
        public static unsafe explicit operator long(IntPtr value)
        {
            return (nint)value._value;
        }

        [NonVersionable]
        public static unsafe bool operator ==(IntPtr value1, IntPtr value2)
        {
            return value1._value == value2._value;
        }

        [NonVersionable]
        public static unsafe bool operator !=(IntPtr value1, IntPtr value2)
        {
            return value1._value != value2._value;
        }

        [NonVersionable]
        public static IntPtr Add(IntPtr pointer, int offset)
        {
            return pointer + offset;
        }

        [NonVersionable]
        public static unsafe IntPtr operator +(IntPtr pointer, int offset)
        {
            return new IntPtr((nint)pointer._value + offset);
        }

        [NonVersionable]
        public static IntPtr Subtract(IntPtr pointer, int offset)
        {
            return pointer - offset;
        }

        [NonVersionable]
        public static unsafe IntPtr operator -(IntPtr pointer, int offset)
        {
            return new IntPtr((nint)pointer._value - offset);
        }

        public static int Size
        {
            [NonVersionable]
            get
            {
                return sizeof(nint);
            }
        }

        [CLSCompliant(false)]
        [NonVersionable]
        public unsafe void* ToPointer()
        {
            return _value;
        }
    }
}
