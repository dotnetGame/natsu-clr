using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using ThreadStart = System.Threading.ThreadStart;
using ThreadState = System.Diagnostics.ThreadState;

namespace Chino.Threading
{
    public class Thread : IThread
    {
        private readonly ThreadStart _start;
        private object? _startArg;

        public ThreadContext Context;
        public volatile Scheduler? Scheduler;

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
                ChipControl.SetThreadDescription(ref Context.Arch, value);
            }
        }

        public Thread(ThreadStart start)
        {
            _start = start ?? throw new ArgumentNullException(nameof(start));
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));

            ChipControl.InitializeThreadContext(ref Context.Arch, this);
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
