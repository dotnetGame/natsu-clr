using System;
using Chino.Chip.K210.HAL.Serial;

namespace Chino.Kernel
{
    public class KernelDebug
    {
        private static void Write(string message)
        {
            Uarths.DebugWrite(message);
        }
    }
}
