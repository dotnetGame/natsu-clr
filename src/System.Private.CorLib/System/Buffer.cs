// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Diagnostics;

using nint = System.Int64;
using nuint = System.UInt64;
using Internal.Runtime.CompilerServices;

namespace System
{
    public static class Buffer
    {
        // Copies from one primitive array to another primitive array without
        // respecting types.  This calls memmove internally.  The count and 
        // offset parameters here are in bytes.  If you want to use traditional
        // array element indices and counts, use Array.Copy.
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void BlockCopy(Array src, int srcOffset,
            Array dst, int dstOffset, int count);

        // Returns a bool to indicate if the array is of primitive data types
        // or not.
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern bool IsPrimitiveTypeArray(Array array);

        // Gets a particular byte out of the array.  The array must be an
        // array of primitives.  
        //
        // This essentially does the following: 
        // return ((byte*)array) + index.
        //
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern byte _GetByte(Array array, int index);

        // Sets a particular byte in an the array.  The array must be an
        // array of primitives.  
        //
        // This essentially does the following: 
        // *(((byte*)array) + index) = value.
        //
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void _SetByte(Array array, int index, byte value);


        // Gets a particular byte out of the array.  The array must be an
        // array of primitives.  
        //
        // This essentially does the following: 
        // return array.length * sizeof(array.UnderlyingElementType).
        //
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern int _ByteLength(Array array);

        internal static unsafe void ZeroMemory(byte* src, long len)
        {
            while (len-- > 0)
                *(src + len) = 0;
        }

        // The attributes on this method are chosen for best JIT performance. 
        // Please do not edit unless intentional.
        [MethodImplAttribute(MethodImplOptions.AggressiveInlining)]
        [CLSCompliant(false)]
        public static unsafe void MemoryCopy(void* source, void* destination, long destinationSizeInBytes, long sourceBytesToCopy)
        {
            if (sourceBytesToCopy > destinationSizeInBytes)
            {
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.sourceBytesToCopy);
            }
            Memmove((byte*)destination, (byte*)source, checked((nuint)sourceBytesToCopy));
        }

        // The attributes on this method are chosen for best JIT performance. 
        // Please do not edit unless intentional.
        [MethodImplAttribute(MethodImplOptions.AggressiveInlining)]
        [CLSCompliant(false)]
        public static unsafe void MemoryCopy(void* source, void* destination, ulong destinationSizeInBytes, ulong sourceBytesToCopy)
        {
            if (sourceBytesToCopy > destinationSizeInBytes)
            {
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.sourceBytesToCopy);
            }
            Memmove((byte*)destination, (byte*)source, checked((nuint)sourceBytesToCopy));
        }

        internal static unsafe void Memcpy(byte[] dest, int destIndex, byte* src, int srcIndex, int len)
        {
            //Debug.Assert((srcIndex >= 0) && (destIndex >= 0) && (len >= 0), "Index and length must be non-negative!");
            //Debug.Assert(dest.Length - destIndex >= len, "not enough bytes in dest");
            // If dest has 0 elements, the fixed statement will throw an 
            // IndexOutOfRangeException.  Special-case 0-byte copies.
            if (len == 0)
                return;
            fixed (byte* pDest = dest)
            {
                Memcpy(pDest + destIndex, src + srcIndex, len);
            }
        }

        // This method has different signature for x64 and other platforms and is done for performance reasons.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Memmove<T>(ref T destination, ref T source, nuint elementCount)
        {
            Memmove(
                ref Unsafe.As<T, byte>(ref destination),
                ref Unsafe.As<T, byte>(ref source),
                elementCount * (nuint)Unsafe.SizeOf<T>());
        }

        internal static unsafe void Memcpy(byte* pDest, int destIndex, byte[] src, int srcIndex, int len)
        {
            //Debug.Assert((srcIndex >= 0) && (destIndex >= 0) && (len >= 0), "Index and length must be non-negative!");
            //Debug.Assert(src.Length - srcIndex >= len, "not enough bytes in src");
            // If dest has 0 elements, the fixed statement will throw an 
            // IndexOutOfRangeException.  Special-case 0-byte copies.
            if (len == 0)
                return;
            fixed (byte* pSrc = src)
            {
                Memcpy(pDest + destIndex, pSrc + srcIndex, len);
            }
        }

        // Non-inlinable wrapper around the QCall that avoids polluting the fast path
        // with P/Invoke prolog/epilog.
        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        private static unsafe void Memmove(ref byte dest, ref byte src, nuint len)
        {
            fixed (byte* pDest = &dest)
            fixed (byte* pSrc = &src)
                Memmove(pDest, pSrc, len);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern unsafe void Memcpy(byte* dest, byte* src, int len);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern unsafe void Memmove(byte* dest, byte* src, nuint len);
    }
}
