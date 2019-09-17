using BitFields;

namespace Chino.Chip.K210.HAL.Serial
{
    partial struct uarths_txdata
    {
        public uint Value;

        private const int dataShift = 0;
        private const uint dataMask = unchecked((uint)((1U << 8) - (1U << 0)));
        public Bit8 data
        {
            get => (Bit8)((Value & dataMask) >> dataShift);
            set => Value = unchecked((uint)((Value & ~dataMask) | ((((uint)value) << dataShift) & dataMask)));
        }
        private const int zeroShift = 8;
        private const uint zeroMask = unchecked((uint)((1U << 31) - (1U << 8)));
        public Bit23 zero
        {
            get => (Bit23)((Value & zeroMask) >> zeroShift);
            set => Value = unchecked((uint)((Value & ~zeroMask) | ((((uint)value) << zeroShift) & zeroMask)));
        }
        private const int fullShift = 31;
        private const uint fullMask = unchecked((uint)((1U << 32) - (1U << 31)));
        public Bit1 full
        {
            get => (Bit1)((Value & fullMask) >> fullShift);
            set => Value = unchecked((uint)((Value & ~fullMask) | ((((uint)value) << fullShift) & fullMask)));
        }
    }
}
