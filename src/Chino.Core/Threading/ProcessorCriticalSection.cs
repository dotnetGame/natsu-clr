using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chip;

namespace Chino.Threading
{
    public struct ProcessorCriticalSection : IDisposable
    {
        private UIntPtr _lastState;

        public static ProcessorCriticalSection Acquire()
        {
            return new ProcessorCriticalSection { _lastState = ChipControl.Default.DisableInterrupt() };
        }

        public void Dispose()
        {
            ChipControl.Default.RestoreInterrupt(_lastState);
        }
    }
}
