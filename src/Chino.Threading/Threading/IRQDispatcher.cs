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

    public static class IRQDispatcher
    {
        private static readonly SystemIRQHandler?[] _systemIRQHandlers = new SystemIRQHandler?[(int)SystemIRQ.COUNT];
        private static readonly LinkedList<DPC> _dpcs = new LinkedList<DPC>();

        static IRQDispatcher()
        {
            _systemIRQHandlers[(int)SystemIRQ.CoreNotification] = OnCoreNotification;
        }

        public static void RegisterSystemIRQ(SystemIRQ irq, SystemIRQHandler? handler)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            Volatile.Write(ref _systemIRQHandlers[(int)irq], handler);
        }

        public static void RegisterDPC(LinkedListNode<DPC> dpc)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _dpcs.AddLast(dpc);
            }

            ChipControl.Default.RaiseCoreNotification();
        }

        internal static void DispatchSystemIRQ(SystemIRQ irq, ThreadContext context)
        {
            Debug.Assert(irq < SystemIRQ.COUNT);
            var handler = Volatile.Read(ref _systemIRQHandlers[(int)irq]);
            if (handler != null)
                context = handler(irq, context);
            else
                UnhandledIRQ(irq);

            ExitIRQHandler(context);
        }

        private static void ExitIRQHandler(ThreadContext context)
        {
            ChipControl.Default.RestoreContext(context);
        }

        private static ThreadContext OnCoreNotification(SystemIRQ irq, ThreadContext context)
        {
            while (_dpcs.Count != 0)
            {
                var item = _dpcs.First;
                _dpcs.RemoveFirst();

                context = item!.Value.Callback(item.Value.Argument, context);
            }

            return context;
        }

        private static void UnhandledIRQ(SystemIRQ irq)
        {
            Debug.Fail("Unhandled IRQ");
        }
    }
}
