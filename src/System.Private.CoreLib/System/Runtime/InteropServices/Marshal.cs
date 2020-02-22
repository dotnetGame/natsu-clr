// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Runtime.CompilerServices;
using System.Diagnostics;

namespace System.Runtime.InteropServices
{
    public enum CustomQueryInterfaceMode
    {
        Ignore = 0,
        Allow = 1
    }

    /// <summary>
    /// This class contains methods that are mainly used to marshal between unmanaged
    /// and managed types.
    /// </summary>
    public static partial class Marshal
    {
#if FEATURE_COMINTEROP
        internal static Guid IID_IUnknown = new Guid("00000000-0000-0000-C000-000000000046");
#endif //FEATURE_COMINTEROP

        private const int LMEM_FIXED = 0;
        private const int LMEM_MOVEABLE = 2;
#if !FEATURE_PAL
        private const long HiWordMask = unchecked((long)0xffffffffffff0000L);
#endif //!FEATURE_PAL

        // Win32 has the concept of Atoms, where a pointer can either be a pointer
        // or an int.  If it's less than 64K, this is guaranteed to NOT be a 
        // pointer since the bottom 64K bytes are reserved in a process' page table.
        // We should be careful about deallocating this stuff.  Extracted to
        // a function to avoid C# problems with lack of support for IntPtr.
        // We have 2 of these methods for slightly different semantics for NULL.
        private static bool IsWin32Atom(IntPtr ptr)
        {
#if FEATURE_PAL
            return false;
#else
            long lPtr = (long)ptr;
            return 0 == (lPtr & HiWordMask);
#endif
        }

        /// <summary>
        /// The default character size for the system. This is always 2 because
        /// the framework only runs on UTF-16 systems.
        /// </summary>
        public static readonly int SystemDefaultCharSize = 2;

        public static void Copy(int[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(char[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(short[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(long[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(float[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(double[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(byte[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        public static void Copy(IntPtr[] source, int startIndex, IntPtr destination, int length)
        {
            CopyToNative(source, startIndex, destination, length);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CopyToNative(object source, int startIndex, IntPtr destination, int length);

        public static void Copy(IntPtr source, int[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, char[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, short[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, long[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, float[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, double[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, byte[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        public static void Copy(IntPtr source, IntPtr[] destination, int startIndex, int length)
        {
            CopyToManaged(source, destination, startIndex, length);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CopyToManaged(IntPtr source, object destination, int startIndex, int length);
    }
}
