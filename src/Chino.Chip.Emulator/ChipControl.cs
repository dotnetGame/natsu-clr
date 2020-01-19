using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Chino.Chip;
using Chino.Threading;

namespace Chino
{
    public static class ChipControl
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Initialize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Write(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern UIntPtr EnableInterrupt();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern UIntPtr DisableInterrupt();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RestoreInterrupt(UIntPtr state);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void InitializeThreadContext(ref ThreadContextArch context, IThread thread);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StartSchedule(in ThreadContextArch context);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetupSystemTimer(TimeSpan timeSlice);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void RestoreContext(in ThreadContextArch context);
    }
}
