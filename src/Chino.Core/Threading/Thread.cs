using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Chino.Chip;
using ThreadStart = System.Threading.ThreadStart;
using ThreadState = System.Diagnostics.ThreadState;

namespace Chino.Threading
{
    public class Thread
    {
        private readonly ThreadStart _start;
        private object? _startArg;

        public volatile Scheduler? Scheduler;

        public ThreadContext Context { get; }

        public LinkedListNode<ThreadScheduleEntry> ScheduleEntry { get; }

        public int ExitCode { get; private set; }

        public ThreadState State { get; internal set; } = ThreadState.Initialized;

        private string? _description;
        public string? Description
        {
            get => _description;
            set
            {
                _description = value;
                ChipControl.Default.SetThreadDescription(Context, value);
            }
        }

        public Thread(ThreadStart start)
        {
            _start = start ?? throw new ArgumentNullException(nameof(start));
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));

            Context = ChipControl.Default.InitializeThreadContext(this);
        }

        public void Start(object? arg = null)
        {
            _startArg = arg;
            KernelServices.Scheduler.StartThread(this);
        }

        public void Exit(int exitCode)
        {
            ExitCode = exitCode;
            Scheduler?.KillThread(this);
        }

        private static void ThreadMainThunk(Thread thread)
        {
            thread._start();
            thread.Exit(0);
        }
    }
}
