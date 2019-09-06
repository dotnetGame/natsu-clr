// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Internal.Runtime.CompilerServices;

#if BIT64
using nuint = System.UInt64;
#else
using nuint = System.UInt32;
#endif // BIT64

namespace System
{
    /// <summary>
    /// Extension methods for Span{T}, Memory{T}, and friends.
    /// </summary>
    public static partial class MemoryExtensions
    {
        /// <summary>
        /// Determines whether two sequences are equal by comparing the elements using IEquatable{T}.Equals(T). 
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool SequenceEqual<T>(this Span<T> span, ReadOnlySpan<T> other)
            where T : IEquatable<T>
        {
            int length = span.Length;

            if (default(T) != null && IsTypeComparableAsBytes<T>(out nuint size))
                return length == other.Length &&
                SpanHelpers.SequenceEqual(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(other)),
                    length * (int)size);  // If this multiplication overflows, the Span we got overflows the entire address range. There's no happy outcome for this api in such a case so we choose not to take the overhead of checking.

            return length == other.Length && SpanHelpers.SequenceEqual(ref MemoryMarshal.GetReference(span), ref MemoryMarshal.GetReference(other), length);
        }

        /// <summary>
        /// Determines whether two sequences are equal by comparing the elements using IEquatable{T}.Equals(T). 
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool SequenceEqual<T>(this ReadOnlySpan<T> span, ReadOnlySpan<T> other)
            where T : IEquatable<T>
        {
            int length = span.Length;
            if (default(T) != null && IsTypeComparableAsBytes<T>(out nuint size))
                return length == other.Length &&
                SpanHelpers.SequenceEqual(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(other)),
                    length * (int)size);  // If this multiplication overflows, the Span we got overflows the entire address range. There's no happy outcome for this api in such a case so we choose not to take the overhead of checking.

            return length == other.Length && SpanHelpers.SequenceEqual(ref MemoryMarshal.GetReference(span), ref MemoryMarshal.GetReference(other), length);
        }

        /// <summary>
        /// Searches for the specified value and returns the index of its first occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The value to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int IndexOf<T>(this Span<T> span, T value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, byte>(ref value),
                    span.Length);

            if (typeof(T) == typeof(char))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, char>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, char>(ref value),
                    span.Length);

            return SpanHelpers.IndexOf(ref MemoryMarshal.GetReference(span), value, span.Length);
        }

        /// <summary>
        /// Searches for the specified sequence and returns the index of its first occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The sequence to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int IndexOf<T>(this Span<T> span, ReadOnlySpan<T> value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    span.Length,
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(value)),
                    value.Length);

            return SpanHelpers.IndexOf(ref MemoryMarshal.GetReference(span), span.Length, ref MemoryMarshal.GetReference(value), value.Length);
        }


        /// <summary>
        /// Searches for the specified value and returns the index of its first occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The value to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int IndexOf<T>(this ReadOnlySpan<T> span, T value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, byte>(ref value),
                    span.Length);

            if (typeof(T) == typeof(char))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, char>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, char>(ref value),
                    span.Length);

            return SpanHelpers.IndexOf(ref MemoryMarshal.GetReference(span), value, span.Length);
        }

        /// <summary>
        /// Searches for the specified sequence and returns the index of its first occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The sequence to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int IndexOf<T>(this ReadOnlySpan<T> span, ReadOnlySpan<T> value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.IndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    span.Length,
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(value)),
                    value.Length);

            return SpanHelpers.IndexOf(ref MemoryMarshal.GetReference(span), span.Length, ref MemoryMarshal.GetReference(value), value.Length);
        }

        /// <summary>
        /// Searches for the specified value and returns the index of its last occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The value to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int LastIndexOf<T>(this ReadOnlySpan<T> span, T value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.LastIndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, byte>(ref value),
                    span.Length);

            if (typeof(T) == typeof(char))
                return SpanHelpers.LastIndexOf(
                    ref Unsafe.As<T, char>(ref MemoryMarshal.GetReference(span)),
                    Unsafe.As<T, char>(ref value),
                    span.Length);

            return SpanHelpers.LastIndexOf<T>(ref MemoryMarshal.GetReference(span), value, span.Length);
        }

        /// <summary>
        /// Searches for the specified sequence and returns the index of its last occurrence. If not found, returns -1. Values are compared using IEquatable{T}.Equals(T). 
        /// </summary>
        /// <param name="span">The span to search.</param>
        /// <param name="value">The sequence to search for.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int LastIndexOf<T>(this ReadOnlySpan<T> span, ReadOnlySpan<T> value)
            where T : IEquatable<T>
        {
            if (typeof(T) == typeof(byte))
                return SpanHelpers.LastIndexOf(
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(span)),
                    span.Length,
                    ref Unsafe.As<T, byte>(ref MemoryMarshal.GetReference(value)),
                    value.Length);

            return SpanHelpers.LastIndexOf<T>(ref MemoryMarshal.GetReference(span), span.Length, ref MemoryMarshal.GetReference(value), value.Length);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static bool IsTypeComparableAsBytes<T>(out nuint size)
        {
            if (typeof(T) == typeof(byte) || typeof(T) == typeof(sbyte))
            {
                size = sizeof(byte);
                return true;
            }

            if (typeof(T) == typeof(char) || typeof(T) == typeof(short) || typeof(T) == typeof(ushort))
            {
                size = sizeof(char);
                return true;
            }

            if (typeof(T) == typeof(int) || typeof(T) == typeof(uint))
            {
                size = sizeof(int);
                return true;
            }

            if (typeof(T) == typeof(long) || typeof(T) == typeof(ulong))
            {
                size = sizeof(long);
                return true;
            }

            size = default;
            return false;
        }
    }
}
