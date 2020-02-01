using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Chino.Chip;
using Chino.Threading;

namespace Chino.Chip
{
    public sealed class ArchChipControl : ChipControl
    {
        public override TimeSpan DefaultTimeSlice => TimeSpan.FromMilliseconds(100);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void Initialize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void Write(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern UIntPtr EnableInterrupt();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern UIntPtr DisableInterrupt();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void SetThreadDescription(ThreadContext context, string value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void RestoreInterrupt(UIntPtr state);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern ThreadContext InitializeThreadContext(object thread);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void StartSchedule(ThreadContext context);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void SetupSystemTimer(TimeSpan timeSlice);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void RestoreContext(ThreadContext context);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public override extern void RaiseCoreNotification();
    }
}
