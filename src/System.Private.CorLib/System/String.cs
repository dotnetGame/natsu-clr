using System;
using System.Runtime.CompilerServices;
using System.Runtime.Versioning;

namespace System
{
    public sealed partial class String
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
        public static readonly string Empty;

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

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern String(char c, int count);


        // Returns this string.
        public override string ToString()
        {
            return this;
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

        internal static unsafe void wstrcpy(char* dmem, char* smem, int charCount)
        {
            Buffer.Memmove((byte*)dmem, (byte*)smem, ((uint)charCount) * 2);
        }
    }
}
