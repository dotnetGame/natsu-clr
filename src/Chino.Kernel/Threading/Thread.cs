using System;
using System.Collections.Generic;
using System.Text;
using ThreadStart = System.Threading.ThreadStart;

namespace Chino.Threading
{
    public class Thread : IThread
    {
        private Scheduler _scheduler;
        private readonly ThreadStart _start;

        public ThreadContext Context;

        public LinkedListNode<ThreadScheduleEntry> ScheduleEntry { get; }

        public Thread(ThreadStart start)
        {
            _start = start ?? throw new ArgumentNullException(nameof(start));
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));

            ChipControl.InitializeThreadContext(ref Context.Arch, this);
        }

        public void Start(object? arg)
        {
        }

        private static int ThreadMainThunk(Thread thread)
        {
            thread._start();
            return 0;
        }
    }
}
