using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Chino;

public static partial class Interop
{
    public static partial class IO
    {
        public static extern SafeFileHandle StdinHandle
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        public static extern SafeFileHandle StdoutHandle
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        public static extern SafeFileHandle StderrHandle
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Read(SafeFileHandle handle, Span<byte> buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Write(SafeFileHandle handle, ReadOnlySpan<byte> buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsConsoleHandle(SafeFileHandle handle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool StdinReady();

        public static unsafe int ReadStdin(byte* buffer, int bufferSize)
        {
            return Read(StdinHandle, new Span<byte>(buffer, bufferSize));
        }
    }
}
