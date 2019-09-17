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
    [CLSCompliant(false)]
    [StructLayout(LayoutKind.Sequential)]
    [TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public readonly struct UInt64 : IComparable, IFormattable, IComparable<ulong>, IEquatable<ulong>, ISpanFormattable
    {
        private readonly ulong m_value; // Do not rename (binary serialization)

        public const ulong MaxValue = (ulong)0xffffffffffffffffL;
        public const ulong MinValue = 0x0;

        // Compares this object to another object, returning an integer that
        // indicates the relationship.
        // Returns a value less than zero if this  object
        // null is considered to be less than any instance.
        // If object is not of type UInt64, this method throws an ArgumentException.
        //
        public int CompareTo(object? value)
        {
            if (value == null)
            {
                return 1;
            }

            // Need to use compare because subtraction will wrap
            // to positive for very large neg numbers, etc.
            if (value is ulong i)
            {
                if (m_value < i) return -1;
                if (m_value > i) return 1;
                return 0;
            }

            throw new ArgumentException(SR.Arg_MustBeUInt64);
        }

        public int CompareTo(ulong value)
        {
            // Need to use compare because subtraction will wrap
            // to positive for very large neg numbers, etc.
            if (m_value < value) return -1;
            if (m_value > value) return 1;
            return 0;
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is ulong))
            {
                return false;
            }
            return m_value == ((ulong)obj).m_value;
        }

        [NonVersionable]
        public bool Equals(ulong obj)
        {
            return m_value == obj;
        }

        // The value of the lower 32 bits XORed with the uppper 32 bits.
        public override int GetHashCode()
        {
            return ((int)m_value) ^ (int)(m_value >> 32);
        }

        public override string ToString()
        {
            return Number.FormatUInt64(m_value, null, null);
        }

        public string ToString(IFormatProvider? provider)
        {
            return Number.FormatUInt64(m_value, null, provider);
        }

        public string ToString(string? format)
        {
            return Number.FormatUInt64(m_value, format, null);
        }

        public string ToString(string? format, IFormatProvider? provider)
        {
            return Number.FormatUInt64(m_value, format, provider);
        }

        public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default, IFormatProvider? provider = null)
        {
            return Number.TryFormatUInt64(m_value, format, provider, destination, out charsWritten);
        }

        [CLSCompliant(false)]
        public static ulong Parse(string s)
        {
            if (s == null) ThrowHelper.ThrowArgumentNullException(ExceptionArgument.s);
            return Number.ParseUInt64(s, NumberStyles.Integer, NumberFormatInfo.CurrentInfo);
        }

        [CLSCompliant(false)]
        public static ulong Parse(string s, NumberStyles style)
        {
            NumberFormatInfo.ValidateParseStyleInteger(style);
            if (s == null) ThrowHelper.ThrowArgumentNullException(ExceptionArgument.s);
            return Number.ParseUInt64(s, style, NumberFormatInfo.CurrentInfo);
        }

        [CLSCompliant(false)]
        public static ulong Parse(string s, IFormatProvider? provider)
        {
            if (s == null) ThrowHelper.ThrowArgumentNullException(ExceptionArgument.s);
            return Number.ParseUInt64(s, NumberStyles.Integer, NumberFormatInfo.GetInstance(provider));
        }

        [CLSCompliant(false)]
        public static ulong Parse(string s, NumberStyles style, IFormatProvider? provider)
        {
            NumberFormatInfo.ValidateParseStyleInteger(style);
            if (s == null) ThrowHelper.ThrowArgumentNullException(ExceptionArgument.s);
            return Number.ParseUInt64(s, style, NumberFormatInfo.GetInstance(provider));
        }

        [CLSCompliant(false)]
        public static ulong Parse(ReadOnlySpan<char> s, NumberStyles style = NumberStyles.Integer, IFormatProvider? provider = null)
        {
            NumberFormatInfo.ValidateParseStyleInteger(style);
            return Number.ParseUInt64(s, style, NumberFormatInfo.GetInstance(provider));
        }

        [CLSCompliant(false)]
        public static bool TryParse(string? s, out ulong result)
        {
            if (s == null)
            {
                result = 0;
                return false;
            }

            return Number.TryParseUInt64(s, NumberStyles.Integer, NumberFormatInfo.CurrentInfo, out result);
        }

        [CLSCompliant(false)]
        public static bool TryParse(ReadOnlySpan<char> s, out ulong result)
        {
            return Number.TryParseUInt64(s, NumberStyles.Integer, NumberFormatInfo.CurrentInfo, out result);
        }

        [CLSCompliant(false)]
        public static bool TryParse(string? s, NumberStyles style, IFormatProvider? provider, out ulong result)
        {
            NumberFormatInfo.ValidateParseStyleInteger(style);

            if (s == null)
            {
                result = 0;
                return false;
            }

            return Number.TryParseUInt64(s, style, NumberFormatInfo.GetInstance(provider), out result);
        }

        [CLSCompliant(false)]
        public static bool TryParse(ReadOnlySpan<char> s, NumberStyles style, IFormatProvider? provider, out ulong result)
        {
            NumberFormatInfo.ValidateParseStyleInteger(style);
            return Number.TryParseUInt64(s, style, NumberFormatInfo.GetInstance(provider), out result);
        }

        //
        // IConvertible implementation
        //

        public TypeCode GetTypeCode()
        {
            return TypeCode.UInt64;
        }
    }
}
