using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Chino.Chip;
using Chino.Collections;

namespace Chino.Threading
{
    public sealed class Scheduler : IScheduler
    {
        private readonly LinkedList<ThreadScheduleEntry> _readyThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _suspendedThreads = new LinkedList<ThreadScheduleEntry>();
        private volatile LinkedListNode<ThreadScheduleEntry>? _runningThread = null;
        private volatile LinkedListNode<DPC> _removalThreadDPC;
        private readonly Thread _idleThread;
        private volatile bool _isRunning;

        public TimeSpan TimeSlice { get; private set; } = ChipControl.DefaultTimeSlice;

        public LinkedListNode<ThreadScheduleEntry> RunningThread => _runningThread!;

        public ulong TickCount { get; private set; }

        public Scheduler()
        {
            _removalThreadDPC = new LinkedListNode<DPC>(new DPC { Callback = OnRemoveThreadDPC });
            _idleThread = CreateThread(IdleMain);
            _idleThread.Description = "Idle";
        }

        public void StartThread(Thread thread)
        {
            if (Interlocked.CompareExchange(ref thread.Scheduler, this, null) == null)
            {
                AddThreadToReadyList(thread);
            }
        }

        public Thread CreateThread(System.Threading.ThreadStart start)
        {
            var thread = new Thread(start);
            return thread;
        }

        public void KillThread(Thread thread)
        {
            Debug.Assert(thread.Scheduler == this);
            if (Interlocked.Exchange(ref thread.Scheduler, null) != null)
            {
                RemoveThreadFromReadyList(thread);
            }
        }

        public void Start()
        {
            Debug.Assert(!_isRunning);

            _idleThread.Start();
            Debug.Assert(_readyThreads.First != null);

            _isRunning = true;
            _runningThread = _readyThreads.First;
            KernelServices.IRQDispatcher.RegisterSystemIRQ(SystemIRQ.SystemTick, OnSystemTick);
            ChipControl.SetupSystemTimer(TimeSlice);
            ChipControl.StartSchedule(_runningThread.Value.Thread.Context.Arch);

            // Should not reach here
            while (true) ;
        }

        private void AddThreadToReadyList(Thread thread)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _readyThreads.AddLast(thread.ScheduleEntry);
            }
        }

        private void RemoveThreadFromReadyList(Thread thread)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                if (_runningThread == thread.ScheduleEntry)
                {
                    Debug.Assert(_removalThreadDPC.Value.Argument == null);
                    _removalThreadDPC.Value.Argument = thread.ScheduleEntry;
                }
                else
                {
                    _readyThreads.Remove(thread.ScheduleEntry);
                }
            }
        }

        private ref ThreadContextArch OnRemoveThreadDPC(object argument, ref ThreadContextArch context)
        {
            Debug.Assert(argument != null);
            var thread = (LinkedListNode<ThreadScheduleEntry>)argument;
            _readyThreads.Remove(thread);

            if (_runningThread == thread)
                return ref _readyThreads.First!.Value.Thread.Context.Arch;
            return ref context;
        }

        private ref ThreadContextArch OnSystemTick(SystemIRQ irq, ref ThreadContextArch context)
        {
            Debug.Assert(_runningThread != null);
            Debug.Assert(_readyThreads.First != null);

            var nextThread = _runningThread.Next ?? _readyThreads.First;
            _runningThread = nextThread;
            Debug.Assert(_runningThread != null);
            return ref nextThread.Value.Thread.Context.Arch;
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
