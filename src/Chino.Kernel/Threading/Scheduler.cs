using System;
using System.Collections.Generic;
using System.Diagnostics;
using Chino.Collections;

namespace Chino.Threading
{
    public sealed class Scheduler : IScheduler
    {
        private readonly LinkedList<ThreadScheduleEntry> _readyThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _suspendedThreads = new LinkedList<ThreadScheduleEntry>();
        private LinkedListNode<ThreadScheduleEntry>? _runningThread = null;
        private LinkedListNode<ThreadScheduleEntry>? _selectedThread = null;
        private readonly Thread _idleThread;
        private bool _isRunning;

        public TimeSpan TimeSlice { get; private set; } = TimeSpan.FromMilliseconds(10);

        public ulong TickCount { get; private set; }

        public Scheduler()
        {
            _idleThread = CreateThread(IdleMain);
        }

        public Thread CreateThread(System.Threading.ThreadStart start)
        {
            var thread = new Thread(start);
            AddThreadToReadyList(thread);
            return thread;
        }

        public void Start()
        {
            Debug.Assert(!_isRunning);

            _isRunning = true;
            Debug.Assert(_selectedThread != null);
            _runningThread = _selectedThread;
            ChipControl.SetupSystemTimer(TimeSlice);
            ChipControl.StartSchedule(_selectedThread.Value.Thread.Context.Arch);

            // Should not reach here
            while (true) ;
        }

        private void AddThreadToReadyList(Thread thread)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _readyThreads.AddLast(thread.ScheduleEntry);

                if (_selectedThread == null)
                    _selectedThread = thread.ScheduleEntry;
            }
        }

        private void IdleMain()
        {
            while (true) ;
        }
    }

    public class ThreadScheduleEntry
    {
        public Thread Thread { get; }

        public ThreadScheduleEntry(Thread thread)
        {
            Thread = thread;
        }
    }
}
