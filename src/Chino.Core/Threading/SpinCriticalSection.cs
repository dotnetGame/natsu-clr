using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Chino.Chip;

namespace Chino.Threading
{
    public struct SpinCriticalSection
    {
        private UIntPtr _lastState;
        private int _lockTaken;

        public void Enter()
        {
            _lastState = ChipControl.Default.DisableInterrupt();
            while (Interlocked.CompareExchange(ref _lockTaken, 1, 0) == 0) ;
        }

        public void Exit()
        {
            Volatile.Write(ref _lockTaken, 0);
            ChipControl.Default.RestoreInterrupt(_lastState);
        }
    }
}
