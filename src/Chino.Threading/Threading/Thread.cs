using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Chino.Chip;
using Chino.Objects;
using ThreadStart = System.Threading.ThreadStart;
using ThreadState = System.Diagnostics.ThreadState;

namespace Chino.Threading
{
    public sealed class Thread : Chino.Objects.Object
    {
        private readonly ThreadStart? _start;
        private readonly ParameterizedThreadStart? _paramterizedStart;
        private object? _startArg;

        internal volatile Scheduler? _scheduler;

        internal ThreadContext Context { get; }

        internal LinkedListNode<ThreadScheduleEntry> ScheduleEntry { get; }

        internal int ExitCode { get; private set; }

        internal ThreadState State { get; set; } = ThreadState.Initialized;

        private string? _description;
        internal string? Description
        {
            get => _description;
            set
            {
                _description = value;
                ChipControl.Default.SetThreadDescription(Context, value);
            }
        }

        internal Thread(ThreadStart start)
        {
            _start = start ?? throw new ArgumentNullException(nameof(start));
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));

            Context = ChipControl.Default.InitializeThreadContext(this);
        }

        internal Thread(ParameterizedThreadStart start)
        {
            _paramterizedStart = _paramterizedStart ?? throw new ArgumentNullException(nameof(start));
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));

            Context = ChipControl.Default.InitializeThreadContext(this);
        }

        internal void Start(object? arg = null)
        {
            _startArg = arg;
            Scheduler.Current.StartThread(this);
        }

        internal void Exit(int exitCode)
        {
            if (_scheduler == null)
                throw new InvalidOperationException();

            ExitCode = exitCode;
            _scheduler.KillThread(this);
        }

        private static void ThreadMainThunk(Thread thread)
        {
            try
            {
                if (thread._start != null)
                    thread._start();
                else
                    thread._paramterizedStart!(thread._startArg);

                thread.Exit(0);
            }
            catch (Exception ex)
            {
                thread.Exit(ex.HResult);
            }
        }

        public override bool CanOpen => true;

        protected override void Open(AccessMask grantedAccess)
        {
        }
    }
}
