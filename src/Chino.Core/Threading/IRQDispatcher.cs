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
        CoreNotification,
        COUNT
    }

    public class DPC
    {
        public object? Argument;
        public DPCHandler Callback;
    }

    public delegate ThreadContext DPCHandler(object? argument, ThreadContext context);
    public delegate ThreadContext SystemIRQHandler(SystemIRQ irq, ThreadContext context);

    public class IRQDispatcher
    {
        private readonly SystemIRQHandler?[] _systemIRQHandlers = new SystemIRQHandler?[(int)SystemIRQ.COUNT];
        private readonly LinkedList<DPC> _dpcs = new LinkedList<DPC>();

        public IRQDispatcher()
        {
            RegisterSystemIRQ(SystemIRQ.CoreNotification, OnCoreNotification);
        }

        public void RegisterSystemIRQ(SystemIRQ irq, SystemIRQHandler? handler)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            Volatile.Write(ref _systemIRQHandlers[(int)irq], handler);
        }

        public void RegisterDPC(LinkedListNode<DPC> dpc)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _dpcs.AddLast(dpc);
            }

            ChipControl.Default.RaiseCoreNotification();
        }

        internal void DispatchSystemIRQ(SystemIRQ irq, ThreadContext context)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            var handler = Volatile.Read(ref _systemIRQHandlers[(int)irq]);
            if (handler != null)
                context = handler(irq, context);
            else
                UnhandledIRQ(irq);

            ExitIRQHandler(context);
        }

        private void ExitIRQHandler(ThreadContext context)
        {
            ChipControl.Default.RestoreContext(context);
        }

        private ThreadContext OnCoreNotification(SystemIRQ irq, ThreadContext context)
        {
            while (_dpcs.Count != 0)
            {
                var item = _dpcs.First;
                _dpcs.RemoveFirst();

                context = item!.Value.Callback(item.Value.Argument, context);
            }

            return context;
        }

        private void UnhandledIRQ(SystemIRQ irq)
        {
            Debug.Fail("Unhandled IRQ");
        }
    }
}
