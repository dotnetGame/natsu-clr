using System;
using System.Runtime.CompilerServices;
using Chino.IO.Devices;

namespace Chino.Chip.Emulator.IO.Devices
{
    public class EmulatorConsole : ConsoleDevice
    {
        private UIntPtr _stdIn, _stdOut;

        public override bool CanRead => true;

        public override bool CanWrite => true;

        public EmulatorConsole()
        {
        }

        protected override void OnInstall()
        {
            OpenStdHandles();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected override extern int Read(Span<byte> buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected override extern void Write(ReadOnlySpan<byte> buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void OpenStdHandles();
    }
}
