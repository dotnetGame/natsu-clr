using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Text;

namespace System
{
    public sealed partial class String : IComparable, IEnumerable<char>, IComparable<string>, IEquatable<string>, ICloneable
    {
        //
        // These fields map directly onto the fields in an EE StringObject.  See object.h for the layout.
        //
        [NonSerialized] private int _stringLength;

        // For empty strings, this will be '\0' since
        // strings are both null-terminated and length prefixed
        [NonSerialized] private char _firstChar;

        // The Empty constant holds the empty string value. It is initialized by the EE during startup.
        // It is treated as intrinsic by the JIT as so the static constructor would never run.
        // Leaving it uninitialized would confuse debuggers.
        //
        // We need to call the String constructor so that the compiler doesn't mark this as a literal.
        // Marking this as a literal would mean that it doesn't show up as a field which we can access 
        // from native.
        public static readonly string Empty = "";

        // Gets the character at a specified position.
        //
        [IndexerName("Chars")]
        public extern char this[int index]
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        // Gets the length of this string
        //
        // This is a EE implemented function so that the JIT can recognise it specially
        // and eliminate checks on character fetches in a loop like:
        //        for(int i = 0; i < str.Length; i++) str[i]
        // The actual code generated for this will be one instruction and will be inlined.
        //
        public extern int Length
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string FastAllocateString(int length);

        // String constructors
        // These are special. The implementation methods for these have a different signature from the
        // declared constructors.

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern String(char[] value);

        private static string Ctor(char[] value)
        {
            if (value == null || value.Length == 0)
                return Empty;

            string result = FastAllocateString(value.Length);
            unsafe
            {
                fixed (char* dest = &result._firstChar, source = value)
                    wstrcpy(dest, source, value.Length);
            }
            return result;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern String(char[] value, int startIndex, int length);

        private static string Ctor(char[] value, int startIndex, int length)
        {
            if (value == null)
                throw new ArgumentNullException(nameof(value));

            if (startIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_StartIndex);

            if (length < 0)
                throw new ArgumentOutOfRangeException(nameof(length), SR.ArgumentOutOfRange_NegativeLength);

            if (startIndex > value.Length - length)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);

            if (length == 0)
                return Empty;

            string result = FastAllocateString(length);
            unsafe
            {
                fixed (char* dest = &result._firstChar, source = value)
                    wstrcpy(dest, source + startIndex, length);
            }
            return result;
        }

        [CLSCompliant(false)]
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern unsafe String(char* value);

        private static unsafe string Ctor(char* ptr)
        {
            if (ptr == null)
                return Empty;

            int count = wcslen(ptr);
            if (count == 0)
                return Empty;

            string result = FastAllocateString(count);
            fixed (char* dest = &result._firstChar)
                wstrcpy(dest, ptr, count);
            return result;
        }

        [CLSCompliant(false)]
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern unsafe String(char* value, int startIndex, int length);

        private static unsafe string Ctor(char* ptr, int startIndex, int length)
        {
            if (length < 0)
                throw new ArgumentOutOfRangeException(nameof(length), SR.ArgumentOutOfRange_NegativeLength);

            if (startIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_StartIndex);

            char* pStart = ptr + startIndex;

            // overflow check
            if (pStart < ptr)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_PartialWCHAR);

            if (length == 0)
                return Empty;

            if (ptr == null)
                throw new ArgumentOutOfRangeException(nameof(ptr), SR.ArgumentOutOfRange_PartialWCHAR);

            string result = FastAllocateString(length);
            fixed (char* dest = &result._firstChar)
                wstrcpy(dest, pStart, length);
            return result;
        }

        [CLSCompliant(false)]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern unsafe String(sbyte* value);

        private static unsafe string Ctor(sbyte* value)
        {
            byte* pb = (byte*)value;
            if (pb == null)
                return Empty;

            int numBytes = new ReadOnlySpan<byte>((byte*)value, int.MaxValue).IndexOf<byte>(0);

            // Check for overflow
            if (numBytes < 0)
                throw new ArgumentException(SR.Arg_MustBeNullTerminatedString);

            return CreateStringForSByteConstructor(pb, numBytes);
        }

        [CLSCompliant(false)]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern unsafe String(sbyte* value, int startIndex, int length);
        private static unsafe string Ctor(sbyte* value, int startIndex, int length)
        {
            if (startIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_StartIndex);

            if (length < 0)
                throw new ArgumentOutOfRangeException(nameof(length), SR.ArgumentOutOfRange_NegativeLength);

            if (value == null)
            {
                if (length == 0)
                    return Empty;

                throw new ArgumentNullException(nameof(value));
            }

            byte* pStart = (byte*)(value + startIndex);

            // overflow check
            if (pStart < value)
                throw new ArgumentOutOfRangeException(nameof(value), SR.ArgumentOutOfRange_PartialWCHAR);

            return CreateStringForSByteConstructor(pStart, length);
        }

        // Encoder for String..ctor(sbyte*) and String..ctor(sbyte*, int, int)
        private static unsafe string CreateStringForSByteConstructor(byte* pb, int numBytes)
        {
            Debug.Assert(numBytes >= 0);
            Debug.Assert(pb <= (pb + numBytes));

            if (numBytes == 0)
                return Empty;

#if PLATFORM_WINDOWS
            int numCharsRequired = Interop.Kernel32.MultiByteToWideChar(Interop.Kernel32.CP_ACP, Interop.Kernel32.MB_PRECOMPOSED, pb, numBytes, (char*)null, 0);
            if (numCharsRequired == 0)
                throw new ArgumentException(SR.Arg_InvalidANSIString);

            string newString = FastAllocateString(numCharsRequired);
            fixed (char *pFirstChar = &newString._firstChar)
            {
                numCharsRequired = Interop.Kernel32.MultiByteToWideChar(Interop.Kernel32.CP_ACP, Interop.Kernel32.MB_PRECOMPOSED, pb, numBytes, pFirstChar, numCharsRequired);
            }
            if (numCharsRequired == 0)
                throw new ArgumentException(SR.Arg_InvalidANSIString);
            return newString;
#else
            return Encoding.UTF8.GetString(pb, numBytes);
#endif
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern String(char c, int count);

        private static string Ctor(char c, int count)
        {
            if (count <= 0)
            {
                if (count == 0)
                    return Empty;
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_NegativeCount);
            }

            string result = FastAllocateString(count);

            if (c != '\0') // Fast path null char string
            {
                unsafe
                {
                    fixed (char* dest = &result._firstChar)
                    {
                        uint cc = (uint)((c << 16) | c);
                        uint* dmem = (uint*)dest;
                        if (count >= 4)
                        {
                            count -= 4;
                            do
                            {
                                dmem[0] = cc;
                                dmem[1] = cc;
                                dmem += 2;
                                count -= 4;
                            } while (count >= 0);
                        }
                        if ((count & 2) != 0)
                        {
                            *dmem = cc;
                            dmem++;
                        }
                        if ((count & 1) != 0)
                            ((char*)dmem)[0] = c;
                    }
                }
            }
            return result;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern String(ReadOnlySpan<char> value);

        private static unsafe string Ctor(ReadOnlySpan<char> value)
        {
            if (value.Length == 0)
                return Empty;

            string result = FastAllocateString(value.Length);
            fixed (char* dest = &result._firstChar, src = &MemoryMarshal.GetReference(value))
                wstrcpy(dest, src, value.Length);
            return result;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static implicit operator ReadOnlySpan<char>(string value) =>
            value != null ? new ReadOnlySpan<char>(ref value.GetRawStringData(), value.Length) : default;

        public object Clone()
        {
            return this;
        }

        public static unsafe string Copy(string str)
        {
            if (str == null)
                throw new ArgumentNullException(nameof(str));

            string result = FastAllocateString(str.Length);
            fixed (char* dest = &result._firstChar, src = &str._firstChar)
                wstrcpy(dest, src, str.Length);
            return result;
        }


        // Converts a substring of this string to an array of characters.  Copies the
        // characters of this string beginning at position sourceIndex and ending at
        // sourceIndex + count - 1 to the character array buffer, beginning
        // at destinationIndex.
        //
        public unsafe void CopyTo(int sourceIndex, char[] destination, int destinationIndex, int count)
        {
            if (destination == null)
                throw new ArgumentNullException(nameof(destination));
            if (count < 0)
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_NegativeCount);
            if (sourceIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(sourceIndex), SR.ArgumentOutOfRange_Index);
            if (count > Length - sourceIndex)
                throw new ArgumentOutOfRangeException(nameof(sourceIndex), SR.ArgumentOutOfRange_IndexCount);
            if (destinationIndex > destination.Length - count || destinationIndex < 0)
                throw new ArgumentOutOfRangeException(nameof(destinationIndex), SR.ArgumentOutOfRange_IndexCount);

            fixed (char* src = &_firstChar, dest = destination)
                wstrcpy(dest + destinationIndex, src + sourceIndex, count);
        }

        // Returns the entire string as an array of characters.
        public unsafe char[] ToCharArray()
        {
            if (Length == 0)
                return Array.Empty<char>();

            char[] chars = new char[Length];
            fixed (char* src = &_firstChar, dest = &chars[0])
                wstrcpy(dest, src, Length);
            return chars;
        }

        // Returns a substring of this string as an array of characters.
        //
        public unsafe char[] ToCharArray(int startIndex, int length)
        {
            // Range check everything.
            if (startIndex < 0 || startIndex > Length || startIndex > Length - length)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);

            if (length <= 0)
            {
                if (length == 0)
                    return Array.Empty<char>();
                throw new ArgumentOutOfRangeException(nameof(length), SR.ArgumentOutOfRange_Index);
            }

            char[] chars = new char[length];
            fixed (char* src = &_firstChar, dest = &chars[0])
                wstrcpy(dest, src + startIndex, length);
            return chars;
        }

        [NonVersionable]
        public static bool IsNullOrEmpty(string value)
        {
            // Using 0u >= (uint)value.Length rather than
            // value.Length == 0 as it will elide the bounds check to
            // the first char: value[0] if that is performed following the test
            // for the same test cost.
            // Ternary operator returning true/false prevents redundant asm generation:
            // https://github.com/dotnet/coreclr/issues/914
            return (value == null || 0u >= (uint)value.Length) ? true : false;
        }

        public static bool IsNullOrWhiteSpace(string value)
        {
            if (value == null) return true;

            for (int i = 0; i < value.Length; i++)
            {
                if (!char.IsWhiteSpace(value[i])) return false;
            }

            return true;
        }

        internal ref char GetRawStringData() => ref _firstChar;

        // Helper for encodings so they can talk to our buffer directly
        // stringLength must be the exact size we'll expect
        internal static unsafe string CreateStringFromEncoding(
            byte* bytes, int byteLength, Encoding encoding)
        {
            Debug.Assert(bytes != null);
            Debug.Assert(byteLength >= 0);

            // Get our string length
            int stringLength = encoding.GetCharCount(bytes, byteLength, null);
            Debug.Assert(stringLength >= 0, "stringLength >= 0");

            // They gave us an empty string if they needed one
            // 0 bytelength might be possible if there's something in an encoder
            if (stringLength == 0)
                return Empty;

            string s = FastAllocateString(stringLength);
            fixed (char* pTempChars = &s._firstChar)
            {
                int doubleCheck = encoding.GetChars(bytes, byteLength, pTempChars, stringLength, null);
                Debug.Assert(stringLength == doubleCheck,
                    "Expected encoding.GetChars to return same length as encoding.GetCharCount");
            }

            return s;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static unsafe extern int wcslen(char* ptr);

        // Returns this string.
        public override string ToString()
        {
            return this;
        }

        public CharEnumerator GetEnumerator()
        {
            return new CharEnumerator(this);
        }

        IEnumerator<char> IEnumerable<char>.GetEnumerator()
        {
            return new CharEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return new CharEnumerator(this);
        }

        // This is only intended to be used by char.ToString.
        // It is necessary to put the code in this class instead of Char, since _firstChar is a private member.
        // Making _firstChar internal would be dangerous since it would make it much easier to break String's immutability.
        internal static string CreateFromChar(char c)
        {
            string result = FastAllocateString(1);
            result._firstChar = c;
            return result;
        }

        internal static unsafe void wstrcpy(char* dmem, char* smem, int charCount)
        {
            Buffer.Memmove((byte*)dmem, (byte*)smem, ((uint)charCount) * 2);
        }
    }
}
