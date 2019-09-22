using System;

namespace Chino.Kernel
{
    public class KernelDebug
    {
        private static void Write(string message)
        {
            ChipControl.Write(message);
        }
    }
}
