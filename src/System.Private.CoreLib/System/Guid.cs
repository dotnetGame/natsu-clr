// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Internal.Runtime.CompilerServices;

namespace System
{
    // Represents a Globally Unique Identifier.
    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    [Runtime.Versioning.NonVersionable] // This only applies to field layout
    [TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public partial struct Guid : IFormattable, IComparable, IComparable<Guid>, IEquatable<Guid>, ISpanFormattable
    {
        public static readonly Guid Empty = new Guid();

        ////////////////////////////////////////////////////////////////////////////////
        //  Member variables
        ////////////////////////////////////////////////////////////////////////////////
        private int _a;   // Do not rename (binary serialization)
        private short _b; // Do not rename (binary serialization)
        private short _c; // Do not rename (binary serialization)
        private byte _d;  // Do not rename (binary serialization)
        private byte _e;  // Do not rename (binary serialization)
        private byte _f;  // Do not rename (binary serialization)
        private byte _g;  // Do not rename (binary serialization)
        private byte _h;  // Do not rename (binary serialization)
        private byte _i;  // Do not rename (binary serialization)
        private byte _j;  // Do not rename (binary serialization)
        private byte _k;  // Do not rename (binary serialization)

        ////////////////////////////////////////////////////////////////////////////////
        //  Constructors
        ////////////////////////////////////////////////////////////////////////////////

        // Creates a new guid from an array of bytes.
        public Guid(byte[] b) :
            this(new ReadOnlySpan<byte>(b ?? throw new ArgumentNullException(nameof(b))))
        {
        }

        // Creates a new guid from a read-only span.
        public Guid(ReadOnlySpan<byte> b)
        {
            if ((uint)b.Length != 16)
                throw new ArgumentException(SR.Format(SR.Arg_GuidArrayCtor, "16"), nameof(b));

            _a = b[3] << 24 | b[2] << 16 | b[1] << 8 | b[0];
            _b = (short)(b[5] << 8 | b[4]);
            _c = (short)(b[7] << 8 | b[6]);
            _d = b[8];
            _e = b[9];
            _f = b[10];
            _g = b[11];
            _h = b[12];
            _i = b[13];
            _j = b[14];
            _k = b[15];
        }

        [CLSCompliant(false)]
        public Guid(uint a, ushort b, ushort c, byte d, byte e, byte f, byte g, byte h, byte i, byte j, byte k)
        {
            _a = (int)a;
            _b = (short)b;
            _c = (short)c;
            _d = d;
            _e = e;
            _f = f;
            _g = g;
            _h = h;
            _i = i;
            _j = j;
            _k = k;
        }

        // Creates a new GUID initialized to the value represented by the arguments.
        //
        public Guid(int a, short b, short c, byte[] d)
        {
            if (d == null)
                throw new ArgumentNullException(nameof(d));
            // Check that array is not too big
            if (d.Length != 8)
                throw new ArgumentException(SR.Format(SR.Arg_GuidArrayCtor, "8"), nameof(d));

            _a = a;
            _b = b;
            _c = c;
            _d = d[0];
            _e = d[1];
            _f = d[2];
            _g = d[3];
            _h = d[4];
            _i = d[5];
            _j = d[6];
            _k = d[7];
        }

        // Creates a new GUID initialized to the value represented by the
        // arguments.  The bytes are specified like this to avoid endianness issues.
        public Guid(int a, short b, short c, byte d, byte e, byte f, byte g, byte h, byte i, byte j, byte k)
        {
            _a = a;
            _b = b;
            _c = c;
            _d = d;
            _e = e;
            _f = f;
            _g = g;
            _h = h;
            _i = i;
            _j = j;
            _k = k;
        }

        private enum GuidParseThrowStyle : byte
        {
            None = 0,
            All = 1,
            AllButOverflow = 2
        }

        // This will store the result of the parsing.  And it will eventually be used to construct a Guid instance.
        private struct GuidResult
        {
            internal readonly GuidParseThrowStyle _throwStyle;
            internal Guid _parsedGuid;

            private bool _overflow;
            private string _failureMessageID;
            private object _failureMessageFormatArgument;

            internal GuidResult(GuidParseThrowStyle canThrow) : this()
            {
                _throwStyle = canThrow;
            }

            internal void SetFailure(bool overflow, string failureMessageID)
            {
                _overflow = overflow;
                _failureMessageID = failureMessageID;
                if (_throwStyle != GuidParseThrowStyle.None)
                {
                    throw CreateGuidParseException();
                }
            }

            internal void SetFailure(bool overflow, string failureMessageID, object failureMessageFormatArgument)
            {
                _failureMessageFormatArgument = failureMessageFormatArgument;
                SetFailure(overflow, failureMessageID);
            }

            internal Exception CreateGuidParseException()
            {
                if (_overflow)
                {
                    return _throwStyle == GuidParseThrowStyle.All ?
                        (Exception)new OverflowException(_failureMessageID) :
                        new FormatException(SR.Format_GuidUnrecognized);
                }
                else
                {
                    return new FormatException(_failureMessageID);
                }
            }
        }

        private static ReadOnlySpan<char> EatAllWhitespace(ReadOnlySpan<char> str)
        {
            // Find the first whitespace character.  If there is none, just return the input.
            int i;
            for (i = 0; i < str.Length && !char.IsWhiteSpace(str[i]); i++) ;
            if (i == str.Length)
            {
                return str;
            }

            // There was at least one whitespace.  Copy over everything prior to it to a new array.
            var chArr = new char[str.Length];
            int newLength = 0;
            if (i > 0)
            {
                newLength = i;
                str.Slice(0, i).CopyTo(chArr);
            }

            // Loop through the remaining chars, copying over non-whitespace.
            for (; i < str.Length; i++)
            {
                char c = str[i];
                if (!char.IsWhiteSpace(c))
                {
                    chArr[newLength++] = c;
                }
            }

            // Return the string with the whitespace removed.
            return new ReadOnlySpan<char>(chArr, 0, newLength);
        }

        private static bool IsHexPrefix(ReadOnlySpan<char> str, int i) =>
            i + 1 < str.Length &&
            str[i] == '0' &&
            (str[i + 1] | 0x20) == 'x';

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private void WriteByteHelper(Span<byte> destination)
        {
            destination[0] = (byte)(_a);
            destination[1] = (byte)(_a >> 8);
            destination[2] = (byte)(_a >> 16);
            destination[3] = (byte)(_a >> 24);
            destination[4] = (byte)(_b);
            destination[5] = (byte)(_b >> 8);
            destination[6] = (byte)(_c);
            destination[7] = (byte)(_c >> 8);
            destination[8] = _d;
            destination[9] = _e;
            destination[10] = _f;
            destination[11] = _g;
            destination[12] = _h;
            destination[13] = _i;
            destination[14] = _j;
            destination[15] = _k;
        }

        // Returns an unsigned byte array containing the GUID.
        public byte[] ToByteArray()
        {
            var g = new byte[16];
            WriteByteHelper(g);
            return g;
        }

        // Returns whether bytes are sucessfully written to given span.
        public bool TryWriteBytes(Span<byte> destination)
        {
            if (destination.Length < 16)
                return false;

            WriteByteHelper(destination);
            return true;
        }

        // Returns the guid in "registry" format.
        public override string ToString()
        {
            return ToString("D", null);
        }

        public override int GetHashCode()
        {
            // Simply XOR all the bits of the GUID 32 bits at a time.
            return _a ^ Unsafe.Add(ref _a, 1) ^ Unsafe.Add(ref _a, 2) ^ Unsafe.Add(ref _a, 3);
        }

        // Returns true if and only if the guid represented
        //  by o is the same as this instance.
        public override bool Equals(object o)
        {
            Guid g;
            // Check that o is a Guid first
            if (o == null || !(o is Guid))
                return false;
            else g = (Guid)o;

            // Now compare each of the elements
            return g._a == _a &&
                Unsafe.Add(ref g._a, 1) == Unsafe.Add(ref _a, 1) &&
                Unsafe.Add(ref g._a, 2) == Unsafe.Add(ref _a, 2) &&
                Unsafe.Add(ref g._a, 3) == Unsafe.Add(ref _a, 3);
        }

        public bool Equals(Guid g)
        {
            // Now compare each of the elements
            return g._a == _a &&
                Unsafe.Add(ref g._a, 1) == Unsafe.Add(ref _a, 1) &&
                Unsafe.Add(ref g._a, 2) == Unsafe.Add(ref _a, 2) &&
                Unsafe.Add(ref g._a, 3) == Unsafe.Add(ref _a, 3);
        }

        private int GetResult(uint me, uint them)
        {
            if (me < them)
            {
                return -1;
            }
            return 1;
        }

        public int CompareTo(object value)
        {
            if (value == null)
            {
                return 1;
            }
            if (!(value is Guid))
            {
                throw new ArgumentException(SR.Arg_MustBeGuid, nameof(value));
            }
            Guid g = (Guid)value;

            if (g._a != _a)
            {
                return GetResult((uint)_a, (uint)g._a);
            }

            if (g._b != _b)
            {
                return GetResult((uint)_b, (uint)g._b);
            }

            if (g._c != _c)
            {
                return GetResult((uint)_c, (uint)g._c);
            }

            if (g._d != _d)
            {
                return GetResult(_d, g._d);
            }

            if (g._e != _e)
            {
                return GetResult(_e, g._e);
            }

            if (g._f != _f)
            {
                return GetResult(_f, g._f);
            }

            if (g._g != _g)
            {
                return GetResult(_g, g._g);
            }

            if (g._h != _h)
            {
                return GetResult(_h, g._h);
            }

            if (g._i != _i)
            {
                return GetResult(_i, g._i);
            }

            if (g._j != _j)
            {
                return GetResult(_j, g._j);
            }

            if (g._k != _k)
            {
                return GetResult(_k, g._k);
            }

            return 0;
        }

        public int CompareTo(Guid value)
        {
            if (value._a != _a)
            {
                return GetResult((uint)_a, (uint)value._a);
            }

            if (value._b != _b)
            {
                return GetResult((uint)_b, (uint)value._b);
            }

            if (value._c != _c)
            {
                return GetResult((uint)_c, (uint)value._c);
            }

            if (value._d != _d)
            {
                return GetResult(_d, value._d);
            }

            if (value._e != _e)
            {
                return GetResult(_e, value._e);
            }

            if (value._f != _f)
            {
                return GetResult(_f, value._f);
            }

            if (value._g != _g)
            {
                return GetResult(_g, value._g);
            }

            if (value._h != _h)
            {
                return GetResult(_h, value._h);
            }

            if (value._i != _i)
            {
                return GetResult(_i, value._i);
            }

            if (value._j != _j)
            {
                return GetResult(_j, value._j);
            }

            if (value._k != _k)
            {
                return GetResult(_k, value._k);
            }

            return 0;
        }

        public static bool operator ==(Guid a, Guid b)
        {
            // Now compare each of the elements
            return a._a == b._a &&
                Unsafe.Add(ref a._a, 1) == Unsafe.Add(ref b._a, 1) &&
                Unsafe.Add(ref a._a, 2) == Unsafe.Add(ref b._a, 2) &&
                Unsafe.Add(ref a._a, 3) == Unsafe.Add(ref b._a, 3);
        }

        public static bool operator !=(Guid a, Guid b)
        {
            // Now compare each of the elements
            return a._a != b._a ||
                Unsafe.Add(ref a._a, 1) != Unsafe.Add(ref b._a, 1) ||
                Unsafe.Add(ref a._a, 2) != Unsafe.Add(ref b._a, 2) ||
                Unsafe.Add(ref a._a, 3) != Unsafe.Add(ref b._a, 3);
        }

        public string ToString(string format)
        {
            return ToString(format, null);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static char HexToChar(int a)
        {
            a = a & 0xf;
            return (char)((a > 9) ? a - 10 + 0x61 : a + 0x30);
        }

        private static unsafe int HexsToChars(char* guidChars, int a, int b)
        {
            guidChars[0] = HexToChar(a >> 4);
            guidChars[1] = HexToChar(a);

            guidChars[2] = HexToChar(b >> 4);
            guidChars[3] = HexToChar(b);

            return 4;
        }

        private static unsafe int HexsToCharsHexOutput(char* guidChars, int a, int b)
        {
            guidChars[0] = '0';
            guidChars[1] = 'x';

            guidChars[2] = HexToChar(a >> 4);
            guidChars[3] = HexToChar(a);

            guidChars[4] = ',';
            guidChars[5] = '0';
            guidChars[6] = 'x';

            guidChars[7] = HexToChar(b >> 4);
            guidChars[8] = HexToChar(b);

            return 9;
        }

        // IFormattable interface
        // We currently ignore provider
        public string ToString(string format, IFormatProvider provider)
        {
            if (format == null || format.Length == 0)
                format = "D";

            // all acceptable format strings are of length 1
            if (format.Length != 1)
                throw new FormatException(SR.Format_InvalidGuidFormatSpecification);

            int guidSize;
            switch (format[0])
            {
                case 'D':
                case 'd':
                    guidSize = 36;
                    break;
                case 'N':
                case 'n':
                    guidSize = 32;
                    break;
                case 'B':
                case 'b':
                case 'P':
                case 'p':
                    guidSize = 38;
                    break;
                case 'X':
                case 'x':
                    guidSize = 68;
                    break;
                default:
                    throw new FormatException(SR.Format_InvalidGuidFormatSpecification);
            }

            string guidString = string.FastAllocateString(guidSize);

            int bytesWritten;
            bool result = TryFormat(new Span<char>(ref guidString.GetRawStringData(), guidString.Length), out bytesWritten, format);
            Debug.Assert(result && bytesWritten == guidString.Length, "Formatting guid should have succeeded.");

            return guidString;
        }

        // Returns whether the guid is successfully formatted as a span. 
        public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format = default)
        {
            if (format.Length == 0)
                format = "D";

            // all acceptable format strings are of length 1
            if (format.Length != 1) 
                throw new FormatException(SR.Format_InvalidGuidFormatSpecification);

            bool dash = true;
            bool hex = false;
            int braces = 0;

            int guidSize;

            switch (format[0])
            {
                case 'D':
                case 'd':
                    guidSize = 36;
                    break;
                case 'N':
                case 'n':
                    dash = false;
                    guidSize = 32;
                    break;
                case 'B':
                case 'b':
                    braces = '{' + ('}' << 16);
                    guidSize = 38;
                    break;
                case 'P':
                case 'p':
                    braces = '(' + (')' << 16);
                    guidSize = 38;
                    break;
                case 'X':
                case 'x':
                    braces = '{' + ('}' << 16);
                    dash = false;
                    hex = true;
                    guidSize = 68;
                    break;
                default:
                    throw new FormatException(SR.Format_InvalidGuidFormatSpecification);
            }

            if (destination.Length < guidSize)
            {
                charsWritten = 0;
                return false;
            }

            unsafe
            {
                fixed (char* guidChars = &MemoryMarshal.GetReference(destination))
                {
                    char * p = guidChars;

                    if (braces != 0)
                        *p++ = (char)braces;

                    if (hex)
                    {
                        // {0xdddddddd,0xdddd,0xdddd,{0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd}}
                        *p++ = '0';
                        *p++ = 'x';
                        p += HexsToChars(p, _a >> 24, _a >> 16);
                        p += HexsToChars(p, _a >> 8, _a);
                        *p++ = ',';
                        *p++ = '0';
                        *p++ = 'x';
                        p += HexsToChars(p, _b >> 8, _b);
                        *p++ = ',';
                        *p++ = '0';
                        *p++ = 'x';
                        p += HexsToChars(p, _c >> 8, _c);
                        *p++ = ',';
                        *p++ = '{';
                        p += HexsToCharsHexOutput(p, _d, _e);
                        *p++ = ',';
                        p += HexsToCharsHexOutput(p, _f, _g);
                        *p++ = ',';
                        p += HexsToCharsHexOutput(p, _h, _i);
                        *p++ = ',';
                        p += HexsToCharsHexOutput(p, _j, _k);
                        *p++ = '}';
                    }
                    else
                    {
                        // [{|(]dddddddd[-]dddd[-]dddd[-]dddd[-]dddddddddddd[}|)]
                        p += HexsToChars(p, _a >> 24, _a >> 16);
                        p += HexsToChars(p, _a >> 8, _a);
                        if (dash)
                            *p++ = '-';
                        p += HexsToChars(p, _b >> 8, _b);
                        if (dash)
                            *p++ = '-';
                        p += HexsToChars(p, _c >> 8, _c);
                        if (dash)
                            *p++ = '-';
                        p += HexsToChars(p, _d, _e);
                        if (dash)
                            *p++ = '-';
                        p += HexsToChars(p, _f, _g);
                        p += HexsToChars(p, _h, _i);
                        p += HexsToChars(p, _j, _k);
                    }

                    if (braces != 0)
                        *p++ = (char)(braces >> 16);

                    Debug.Assert(p - guidChars == guidSize);
                }
            }

            charsWritten = guidSize;
            return true;
        }

        bool ISpanFormattable.TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format, IFormatProvider provider)
        {
            // Like with the IFormattable implementation, provider is ignored.
            return TryFormat(destination, out charsWritten, format);
        }
    }
}
