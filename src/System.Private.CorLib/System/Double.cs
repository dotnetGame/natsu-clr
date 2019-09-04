// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
**
**
** Purpose: A representation of an IEEE double precision
**          floating point number.
**
**
===========================================================*/

using System.Runtime.CompilerServices;
using System.Runtime.Versioning;
using Internal.Runtime.CompilerServices;

namespace System
{
    public struct Double
    {
        private double m_value; // Do not rename (binary serialization)

        //
        // Public Constants
        //
        public const double MinValue = -1.7976931348623157E+308;
        public const double MaxValue = 1.7976931348623157E+308;

        // Note Epsilon should be a double whose hex representation is 0x1
        // on little endian machines.
        public const double Epsilon = 4.9406564584124654E-324;
        public const double NegativeInfinity = (double)-1.0 / (double)(0.0);
        public const double PositiveInfinity = (double)1.0 / (double)(0.0);
        public const double NaN = (double)0.0 / (double)0.0;

        // We use this explicit definition to avoid the confusion between 0.0 and -0.0.
        internal const double NegativeZero = -0.0;

        /// <summary>Determines whether the specified value is finite (zero, subnormal, or normal).</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsFinite(double d)
        {
            var bits = BitConverter.DoubleToInt64Bits(d);
            return (bits & 0x7FFFFFFFFFFFFFFF) < 0x7FF0000000000000;
        }

        /// <summary>Determines whether the specified value is infinite.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsInfinity(double d)
        {
            var bits = BitConverter.DoubleToInt64Bits(d);
            return (bits & 0x7FFFFFFFFFFFFFFF) == 0x7FF0000000000000;
        }

        /// <summary>Determines whether the specified value is NaN.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsNaN(double d)
        {
            var bits = BitConverter.DoubleToInt64Bits(d);
            return (bits & 0x7FFFFFFFFFFFFFFF) > 0x7FF0000000000000;
        }

        /// <summary>Determines whether the specified value is negative.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe bool IsNegative(double d)
        {
            return BitConverter.DoubleToInt64Bits(d) < 0;
        }

        /// <summary>Determines whether the specified value is negative infinity.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool IsNegativeInfinity(double d)
        {
            return (d == double.NegativeInfinity);
        }

        /// <summary>Determines whether the specified value is normal.</summary>
        [NonVersionable]
        // This is probably not worth inlining, it has branches and should be rarely called
        public static unsafe bool IsNormal(double d)
        {
            var bits = BitConverter.DoubleToInt64Bits(d);
            bits &= 0x7FFFFFFFFFFFFFFF;
            return (bits < 0x7FF0000000000000) && (bits != 0) && ((bits & 0x7FF0000000000000) != 0);
        }

        /// <summary>Determines whether the specified value is positive infinity.</summary>
        [NonVersionable]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool IsPositiveInfinity(double d)
        {
            return (d == double.PositiveInfinity);
        }

        /// <summary>Determines whether the specified value is subnormal.</summary>
        [NonVersionable]
        // This is probably not worth inlining, it has branches and should be rarely called
        public static unsafe bool IsSubnormal(double d)
        {
            var bits = BitConverter.DoubleToInt64Bits(d);
            bits &= 0x7FFFFFFFFFFFFFFF;
            return (bits < 0x7FF0000000000000) && (bits != 0) && ((bits & 0x7FF0000000000000) == 0);
        }

        // Compares this object to another object, returning an instance of System.Relation.
        // Null is considered less than any instance.
        //
        // If object is not of type Double, this method throws an ArgumentException.
        //
        // Returns a value less than zero if this  object
        //
        public int CompareTo(object value)
        {
            if (value == null)
            {
                return 1;
            }
            if (value is double)
            {
                double d = (double)value;
                if (m_value < d) return -1;
                if (m_value > d) return 1;
                if (m_value == d) return 0;

                // At least one of the values is NaN.
                if (IsNaN(m_value))
                    return (IsNaN(d) ? 0 : -1);
                else
                    return 1;
            }
            throw new ArgumentException(SR.Arg_MustBeDouble);
        }

        public int CompareTo(double value)
        {
            if (m_value < value) return -1;
            if (m_value > value) return 1;
            if (m_value == value) return 0;

            // At least one of the values is NaN.
            if (IsNaN(m_value))
                return (IsNaN(value) ? 0 : -1);
            else
                return 1;
        }

        // True if obj is another Double with the same value as the current instance.  This is
        // a method of object equality, that only returns true if obj is also a double.
        public override bool Equals(object obj)
        {
            if (!(obj is double))
            {
                return false;
            }
            double temp = ((double)obj).m_value;
            // This code below is written this way for performance reasons i.e the != and == check is intentional.
            if (temp == m_value)
            {
                return true;
            }
            return IsNaN(temp) && IsNaN(m_value);
        }

        [NonVersionable]
        public static bool operator ==(double left, double right)
        {
            return left == right;
        }

        [NonVersionable]
        public static bool operator !=(double left, double right)
        {
            return left != right;
        }

        [NonVersionable]
        public static bool operator <(double left, double right)
        {
            return left < right;
        }

        [NonVersionable]
        public static bool operator >(double left, double right)
        {
            return left > right;
        }

        [NonVersionable]
        public static bool operator <=(double left, double right)
        {
            return left <= right;
        }

        [NonVersionable]
        public static bool operator >=(double left, double right)
        {
            return left >= right;
        }

        public bool Equals(double obj)
        {
            if (obj == m_value)
            {
                return true;
            }
            return IsNaN(obj) && IsNaN(m_value);
        }

        //The hashcode for a double is the absolute value of the integer representation
        //of that double.
        //
        [MethodImpl(MethodImplOptions.AggressiveInlining)] // 64-bit constants make the IL unusually large that makes the inliner to reject the method
        public override int GetHashCode()
        {
            var bits = Unsafe.As<double, long>(ref Unsafe.AsRef(in m_value));

            // Optimized check for IsNan() || IsZero()
            if (((bits - 1) & 0x7FFFFFFFFFFFFFFF) >= 0x7FF0000000000000)
            {
                // Ensure that all NaNs and both zeros have the same hash code
                bits &= 0x7FF0000000000000;
            }

            return unchecked((int)bits) ^ ((int)(bits >> 32));
        }
    }
}
