using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Threading
{
    public struct ProcessorCriticalSection : IDisposable
    {
        private UIntPtr _lastState;

        public static ProcessorCriticalSection Acquire()
        {
            return new ProcessorCriticalSection { _lastState = ChipControl.DisableInterrupt() };
        }

        public void Dispose()
        {
            ChipControl.RestoreInterrupt(_lastState);
        }
    }
}
