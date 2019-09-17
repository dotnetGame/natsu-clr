using System;
using System.Text;

namespace Chino.Chip.K210.HAL.Serial
{
    public class Uarths
    {
        private static ref UarthsReg Reg => ref Volatile.As<UarthsReg>(RegisterMap.UARTHS_BASE_ADDR);

        public static void DebugWrite(string text)
        {
            foreach (var c in text)
                DebugWriteChar(c);
        }

        private static void DebugWriteChar(char c)
        {
            DebugWriteByte((byte)c);
        }

        private static void DebugWriteByte(byte value)
        {
            while (Volatile.Read(ref Reg.txdata).full != 0) ;
            Volatile.Write(ref Reg.txdata, new uarths_txdata { data = value });
        }
    }

    internal partial struct uarths_txdata
    {
        enum BitFields { data = 8, zero = 23, full = 1 }
    }

    internal struct UarthsReg
    {
        public uarths_txdata txdata;
    }
}
