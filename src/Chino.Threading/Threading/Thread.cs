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
        internal LinkedListNode<ThreadWaitEntry> WaitEntry { get; }

        internal uint Id { get; }

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

        private Thread(uint id)
        {
            Id = id;
            ScheduleEntry = new LinkedListNode<ThreadScheduleEntry>(new ThreadScheduleEntry(this));
            WaitEntry = new LinkedListNode<ThreadWaitEntry>(new ThreadWaitEntry(this));

            Context = ChipControl.Default.InitializeThreadContext(this);
        }

        internal Thread(uint id, ThreadStart start)
            : this(id)
        {
            _start = start ?? throw new ArgumentNullException(nameof(start));
        }

        internal Thread(uint id, ParameterizedThreadStart start)
            : this(id)
        {
            _paramterizedStart = _paramterizedStart ?? throw new ArgumentNullException(nameof(start));
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

        internal void UnDelay()
        {
            if (_scheduler == null)
                throw new InvalidOperationException();

            _scheduler.UnDelayThread(this);
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
