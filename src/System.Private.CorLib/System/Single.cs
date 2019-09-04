// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
**
**
** Purpose: A wrapper class for the primitive type float.
**
**
===========================================================*/

using System.Runtime.CompilerServices;
using System.Runtime.Versioning;
using Internal.Runtime.CompilerServices;

namespace System
{
    public struct Single
    {
        private float m_value; // Do not rename (binary serialization)

        //
        // Public constants
        //
        public const float MinValue = (float)-3.40282346638528859e+38;
        public const float Epsilon = (float)1.4e-45;
        public const float MaxValue = (float)3.40282346638528859e+38;
        public const float PositiveInfinity = (float)1.0 / (float)0.0;
        public const float NegativeInfinity = (float)-1.0 / (float)0.0;
        public const float NaN = (float)0.0 / (float)0.0;

        // We use this explicit definition to avoid the confusion between 0.0 and -0.0.
        internal const float NegativeZero = (float)-0.0;


        /// <summary>Determines whether the specified value is finite (zero, subnormal, or normal).</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool IsFinite(float f)
        {
            var bits = BitConverter.SingleToInt32Bits(f);
            return (bits & 0x7FFFFFFF) < 0x7F800000;
        }

        /// <summary>Determines whether the specified value is infinite.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsInfinity(float f)
        {
            var bits = BitConverter.SingleToInt32Bits(f);
            return (bits & 0x7FFFFFFF) == 0x7F800000;
        }

        /// <summary>Determines whether the specified value is NaN.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsNaN(float f)
        {
            var bits = BitConverter.SingleToInt32Bits(f);
            return (bits & 0x7FFFFFFF) > 0x7F800000;
        }

        /// <summary>Determines whether the specified value is negative.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsNegative(float f)
        {
            return BitConverter.SingleToInt32Bits(f) < 0;
        }

        /// <summary>Determines whether the specified value is negative infinity.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsNegativeInfinity(float f)
        {
            return (f == float.NegativeInfinity);
        }

        /// <summary>Determines whether the specified value is normal.</summary>
        [NonVersionable]
        // This is probably not worth inlining, it has branches and should be rarely called
        public static unsafe bool IsNormal(float f)
        {
            var bits = BitConverter.SingleToInt32Bits(f);
            bits &= 0x7FFFFFFF;
            return (bits < 0x7F800000) && (bits != 0) && ((bits & 0x7F800000) != 0);
        }

        /// <summary>Determines whether the specified value is positive infinity.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsPositiveInfinity(float f)
        {
            return (f == float.PositiveInfinity);
        }

        /// <summary>Determines whether the specified value is subnormal.</summary>
        [NonVersionable]
        // This is probably not worth inlining, it has branches and should be rarely called
        public static unsafe bool IsSubnormal(float f)
        {
            var bits = BitConverter.SingleToInt32Bits(f);
            bits &= 0x7FFFFFFF;
            return (bits < 0x7F800000) && (bits != 0) && ((bits & 0x7F800000) == 0);
        }

        // Compares this object to another object, returning an integer that
        // indicates the relationship.
        // Returns a value less than zero if this  object
        // null is considered to be less than any instance.
        // If object is not of type Single, this method throws an ArgumentException.
        //
        public int CompareTo(object value)
        {
            if (value == null)
            {
                return 1;
            }
            if (value is float)
            {
                float f = (float)value;
                if (m_value < f) return -1;
                if (m_value > f) return 1;
                if (m_value == f) return 0;

                // At least one of the values is NaN.
                if (IsNaN(m_value))
                    return (IsNaN(f) ? 0 : -1);
                else // f is NaN.
                    return 1;
            }
            throw new ArgumentException(SR.Arg_MustBeSingle);
        }


        public int CompareTo(float value)
        {
            if (m_value < value) return -1;
            if (m_value > value) return 1;
            if (m_value == value) return 0;

            // At least one of the values is NaN.
            if (IsNaN(m_value))
                return (IsNaN(value) ? 0 : -1);
            else // f is NaN.
                return 1;
        }

        [NonVersionable]
        public static bool operator ==(float left, float right)
        {
            return left == right;
        }

        [NonVersionable]
        public static bool operator !=(float left, float right)
        {
            return left != right;
        }

        [NonVersionable]
        public static bool operator <(float left, float right)
        {
            return left < right;
        }

        [NonVersionable]
        public static bool operator >(float left, float right)
        {
            return left > right;
        }

        [NonVersionable]
        public static bool operator <=(float left, float right)
        {
            return left <= right;
        }

        [NonVersionable]
        public static bool operator >=(float left, float right)
        {
            return left >= right;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is float))
            {
                return false;
            }
            float temp = ((float)obj).m_value;
            if (temp == m_value)
            {
                return true;
            }

            return IsNaN(temp) && IsNaN(m_value);
        }

        public bool Equals(float obj)
        {
            if (obj == m_value)
            {
                return true;
            }

            return IsNaN(obj) && IsNaN(m_value);
        }

        public override int GetHashCode()
        {
            var bits = Unsafe.As<float, int>(ref Unsafe.AsRef(in m_value));

            // Optimized check for IsNan() || IsZero()
            if (((bits - 1) & 0x7FFFFFFF) >= 0x7F800000)
            {
                // Ensure that all NaNs and both zeros have the same hash code
                bits &= 0x7F800000;
            }

            return bits;
        }
    }
}
