using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using Chino.Chip;

namespace Chino.Threading
{
    public enum SystemIRQ : uint
    {
        SystemTick,
        COUNT
    }

    public delegate void SystemIRQHandler(SystemIRQ irq, in ThreadContextArch context);

    public class IRQDispatcher
    {
        private readonly SystemIRQHandler?[] _systemIRQHandlers = new SystemIRQHandler?[(int)SystemIRQ.COUNT];

        public void RegisterSystemIRQ(SystemIRQ irq, SystemIRQHandler? handler)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            Volatile.Write(ref _systemIRQHandlers[(int)irq], handler);
        }

        internal void DispatchSystemIRQ(SystemIRQ irq, in ThreadContextArch context)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            var handler = Volatile.Read(ref _systemIRQHandlers[(int)irq]);
            if (handler != null)
                handler(irq, context);
            else
                UnhandledIRQ(irq);

            ExitIRQHandler();
        }

        private void ExitIRQHandler()
        {
            ChipControl.RestoreContext(KernelServices.Scheduler.SelectedThread.Value.Thread.Context.Arch);
        }

        private void UnhandledIRQ(SystemIRQ irq)
        {
            Debug.Fail("Unhandled IRQ");
        }
    }
}
