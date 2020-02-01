using System;
using System.Collections.Generic;
using System.Text;
using Chino.Threading;

namespace Chino
{
    public static class KernelServices
    {
        public static Scheduler Scheduler { get; private set; }

        public static IRQDispatcher IRQDispatcher { get; private set; }

        public static Thread CurrentThread => Scheduler.RunningThread.Value.Thread;

        public static void Initialize()
        {
            Scheduler = new Scheduler();
            IRQDispatcher = new IRQDispatcher();
        }
    }
}
