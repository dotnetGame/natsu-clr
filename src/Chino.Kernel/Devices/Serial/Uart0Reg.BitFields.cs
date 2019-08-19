using BitFields;

namespace Chino.Devices.Serial
{
    partial struct Uart0Reg
    {
        public volatile byte Value;

        private const int AShift = 0;
        private const byte AMask = unchecked((byte)((1U << 1) - (1U << 0)));
        public Bit1 A
        {
            get => (Bit1)((Value & AMask) >> AShift);
            set => Value = unchecked((byte)((Value & ~AMask) | ((((byte)value) << AShift) & AMask)));
        }
    }
}
