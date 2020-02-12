using Chino.Threading;
using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Chip
{
    public abstract class ChipControl
    {
        public static ChipControl Default { get; set; }

        public abstract TimeSpan DefaultTimeSlice { get; }

        public abstract int ProcessorsCount { get; }

        public abstract int CurrentProcessorId { get; }

        public abstract void Initialize();
        public abstract void Write(string message);

        public abstract UIntPtr EnableInterrupt();
        public abstract UIntPtr DisableInterrupt();
        public abstract void RestoreInterrupt(UIntPtr state);

        public abstract ThreadContext InitializeThreadContext(object thread);
        public abstract void UninitializeThreadContext(ThreadContext context);
        public abstract void SetThreadDescription(ThreadContext context, string? description);

        public abstract void StartSchedule(ThreadContext context);
        public abstract void RestoreContext(ThreadContext context);
        public abstract void SetupSystemTimer(TimeSpan timeSlice);

        public abstract void RaiseCoreNotification();

        public virtual void InstallDrivers()
        {
        }

        public virtual void RegisterDeviceDescriptions()
        {
        }
    }
}
