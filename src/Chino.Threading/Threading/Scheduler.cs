using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Threading;
using Chino.Chip;
using Chino.Collections;
using Chino.Objects;
using ThreadState = System.Diagnostics.ThreadState;

namespace Chino.Threading
{
    public sealed class Scheduler
    {
        private static readonly Scheduler[] _schedulers = new Scheduler[ChipControl.Default.ProcessorsCount];

        internal static Scheduler Current => _schedulers[ChipControl.Default.CurrentProcessorId];

        public static Accessor<Thread> CurrentThread => ObjectManager.OpenObject(Current.RunningThread.Value.Thread, AccessMask.GenericAll);

        private readonly LinkedList<ThreadScheduleEntry> _readyThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _delayedThreads = new LinkedList<ThreadScheduleEntry>();
        private readonly LinkedList<ThreadScheduleEntry> _suspendedThreads = new LinkedList<ThreadScheduleEntry>();
        private volatile LinkedListNode<ThreadScheduleEntry>? _runningThread = null;
        private volatile LinkedListNode<DPC> _yieldDPC;
        private readonly Thread _idleThread;
        private volatile bool _isRunning;

        public int Id { get; }

        public TimeSpan TimeSlice { get; private set; } = ChipControl.Default.DefaultTimeSlice;

        internal LinkedListNode<ThreadScheduleEntry> RunningThread => _runningThread!;

        public ulong TickCount { get; private set; }

        static Scheduler()
        {
            for (int i = 0; i < _schedulers.Length; i++)
                _schedulers[i] = new Scheduler(i);
        }

        internal Scheduler(int id)
        {
            Id = id;
            _yieldDPC = new LinkedListNode<DPC>(new DPC { Callback = OnYieldDPC });
            _idleThread = new Thread(IdleMain);
            _idleThread.Description = "Idle";
        }

        public static Accessor<Thread> CreateThread(ThreadStart start)
        {
            return ObjectManager.CreateObject(new Thread(start), AccessMask.GenericAll, ObjectAttributes.Empty);
        }

        public static void Delay(TimeSpan delay)
        {
            Current.DelayCurrentThread(delay);
        }

        public static void ExitThread(int exitCode)
        {
            Current.RunningThread.Value.Thread.Exit(exitCode);
        }

        [DoesNotReturn]
        public static void StartCurrentScheduler()
        {
            Current.Start();
        }

        internal void StartThread(Thread thread)
        {
            if (Interlocked.CompareExchange(ref thread._scheduler, this, null) == null)
            {
                AddThreadToReadyList(thread);
            }
        }

        internal void KillThread(Thread thread)
        {
            Debug.Assert(thread._scheduler == this);
            if (Interlocked.Exchange(ref thread._scheduler, null) != null)
            {
                RemoveThreadFromReadyList(thread);
                thread.State = ThreadState.Terminated;
            }
        }

        private void DelayCurrentThread(TimeSpan delay)
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

                IRQDispatcher.RegisterDPC(_yieldDPC);
            }
        }

        [DoesNotReturn]
        private void Start()
        {
            Debug.Assert(!_isRunning);

            _idleThread.Start();
            Debug.Assert(_readyThreads.First != null);

            _isRunning = true;
            _runningThread = _readyThreads.First;
            IRQDispatcher.RegisterSystemIRQ(SystemIRQ.SystemTick, OnSystemTick);
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
                    IRQDispatcher.RegisterDPC(_yieldDPC);
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

    internal class ThreadScheduleEntry
    {
        public Thread Thread { get; }

        public ulong AwakeTick { get; set; }

        public ThreadScheduleEntry(Thread thread)
        {
            Thread = thread;
        }
    }
}
