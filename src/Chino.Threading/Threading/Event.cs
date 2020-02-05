using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Chino.Objects;

namespace Chino.Threading
{
    public sealed class Event : WaitableObject
    {
        private int _raised;
        private readonly bool _autoReset;
        private readonly LinkedList<ThreadWaitEntry> _waitQueue;
        private readonly SpinCriticalSection _spinCS;

        internal Event(bool initialState = false, bool autoReset = true)
        {
            _waitQueue = new LinkedList<ThreadWaitEntry>();
            _raised = initialState ? 1 : 0;
            _autoReset = autoReset;
        }

        internal void WaitOne(TimeSpan timeout)
        {
            bool needWait = false;
            if (!TestSignal())
            {
                try
                {
                    _spinCS.Enter();
                    if (!TestSignal())
                    {
                        needWait = true;
                        var waitEntry = Scheduler.Current.RunningThread.Value.Thread.WaitEntry;
                        _waitQueue.AddLast(waitEntry);
                        Scheduler.Delay(timeout);
                    }
                }
                finally
                {
                    _spinCS.Exit();
                }
            }

            if (needWait)
            {
                var waitEntry = Scheduler.Current.RunningThread.Value.Thread.WaitEntry;
                if (!waitEntry.Value.Signaled)
                    throw new TimeoutException();
                else
                    waitEntry.Value.Signaled = false;
            }
        }

        internal void SetEvent()
        {
            try
            {
                _spinCS.Enter();

                if (_waitQueue.Count == 0)
                {
                    Volatile.Write(ref _raised, 1);
                }
                else
                {
                    if (_autoReset)
                    {
                        var waitEntry = _waitQueue.First!.Value;
                        _waitQueue.RemoveFirst();
                        waitEntry.Signaled = true;
                        waitEntry.Thread.UnDelay();
                    }
                    else
                    {
                        Volatile.Write(ref _raised, 1);
                        foreach (var waitEntry in _waitQueue)
                        {
                            waitEntry.Signaled = true;
                            waitEntry.Thread.UnDelay();
                        }

                        _waitQueue.Clear();
                    }
                }
            }
            finally
            {
                _spinCS.Exit();
            }
        }

        internal void ResetEvent()
        {
            Volatile.Write(ref _raised, 0);
        }

        private bool TestSignal()
        {
            if (_autoReset)
            {
                if (Interlocked.Exchange(ref _raised, 0) == 1)
                    return true;
            }
            else
            {
                if (Volatile.Read(ref _raised) == 1)
                    return true;
            }

            return false;
        }
    }
}
