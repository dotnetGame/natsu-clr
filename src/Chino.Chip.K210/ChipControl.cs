using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chip.K210.HAL.Serial;

namespace Chino
{
    public static class ChipControl
    {
        public static void Initialize()
        {
        }

        public static void Write(string message)
        {
            Uarths.DebugWrite(message);
        }
    }
}
