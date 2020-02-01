using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Chino.Chip;
using Chino.Collections;
using ThreadState = System.Diagnostics.ThreadState;

namespace Chino.Threading
{
    public sealed class Scheduler
    {
        private readonly LinkedList<ThreadScheduleEntry> _readyThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _delayedThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _suspendedThreads = new LinkedList<ThreadScheduleEntry>();
        private volatile LinkedListNode<ThreadScheduleEntry>? _runningThread = null;
        private volatile LinkedListNode<DPC> _yieldDPC;
        private readonly Thread _idleThread;
        private volatile bool _isRunning;

        public TimeSpan TimeSlice { get; private set; } = ChipControl.Default.DefaultTimeSlice;

        public LinkedListNode<ThreadScheduleEntry> RunningThread => _runningThread!;

        public ulong TickCount { get; private set; }

        public Scheduler()
        {
            _yieldDPC = new LinkedListNode<DPC>(new DPC { Callback = OnYieldDPC });
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

        public Thread CreateThread(ThreadStart start)
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
                thread.State = ThreadState.Terminated;
            }
        }

        public void DelayCurrentThread(TimeSpan delay)
        {
            if (delay.Ticks < 0)
                throw new ArgumentOutOfRangeException(nameof(delay));

            using (ProcessorCriticalSection.Acquire())
            {
                // Yield
                if (delay.Ticks != 0)
                {
                    var thread = _runningThread!;
                    _readyThreads.Remove(thread);
                    thread.Value.Thread.State = ThreadState.Wait;

                    if (delay == Timeout.InfiniteTimeSpan)
                    {
                        _suspendedThreads.AddLast(thread);
                    }
                    else
                    {
                        var awakeTick = TickCount + (ulong)TimeSpanToTicks(delay);
                        thread.Value.AwakeTick = awakeTick;
                        AddThreadToDelayedList(thread);
                    }
                }

                KernelServices.IRQDispatcher.RegisterDPC(_yieldDPC);
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
            ChipControl.Default.SetupSystemTimer(TimeSlice);
            ChipControl.Default.StartSchedule(_runningThread.Value.Thread.Context);

            // Should not reach here
            while (true) ;
        }

        private void AddThreadToReadyList(Thread thread)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _readyThreads.AddLast(thread.ScheduleEntry);
                thread.State = ThreadState.Ready;
            }
        }

        private void AddThreadToDelayedList(LinkedListNode<ThreadScheduleEntry> thread)
        {
            LinkedListNode<ThreadScheduleEntry>? upperBound = null;
            for (var node = _delayedThreads.First; node != null; node = node.Next)
            {
                if (node.Value.AwakeTick > thread.Value.AwakeTick)
                {
                    upperBound = node;
                    break;
                }
            }

            if (upperBound == null)
                _delayedThreads.AddLast(thread);
            else
                _delayedThreads.AddBefore(upperBound, thread);
        }

        private void RemoveThreadFromReadyList(Thread thread)
        {
            using (ProcessorCriticalSection.Acquire())
            {
                _readyThreads.Remove(thread.ScheduleEntry);

                if (_runningThread == thread.ScheduleEntry)
                    KernelServices.IRQDispatcher.RegisterDPC(_yieldDPC);
            }
        }

        private ThreadContext OnYieldDPC(object? argument, ThreadContext context)
        {
            return YieldThread();
        }

        private ThreadContext OnSystemTick(SystemIRQ irq, ThreadContext context)
        {
            TickCount++;

            // Unblock delayed threads
            while (true)
            {
                var first = _delayedThreads.First;
                if (first != null && TickCount >= first.Value.AwakeTick)
                {
                    _delayedThreads.Remove(first);
                    _readyThreads.AddLast(first);
                    first.Value.Thread.State = ThreadState.Ready;
                }
                else
                {
                    break;
                }
            }

            return YieldThread();
        }

        private ThreadContext YieldThread()
        {
            Debug.Assert(_runningThread != null);
            Debug.Assert(_readyThreads.First != null);

            var oldRunningThread = _runningThread;
            if (oldRunningThread.Value.Thread.State == ThreadState.Running)
                oldRunningThread.Value.Thread.State = ThreadState.Ready;

            var nextThread = oldRunningThread.List == _readyThreads
                ? oldRunningThread.Next ?? _readyThreads.First
                : _readyThreads.First;
            _runningThread = nextThread;
            Debug.Assert(_runningThread != null);
            _runningThread.Value.Thread.State = ThreadState.Running;
            return nextThread.Value.Thread.Context;
        }

        private long TimeSpanToTicks(in TimeSpan timeSpan)
        {
            return (long)Math.Ceiling(timeSpan / TimeSlice);
        }

        private void IdleMain()
        {
            while (true) ;
        }
    }

    public class ThreadScheduleEntry
    {
        public Thread Thread { get; }

        public ulong AwakeTick { get; set; }

        public ThreadScheduleEntry(Thread thread)
        {
            Thread = thread;
        }
    }
}
