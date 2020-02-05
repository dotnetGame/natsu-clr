using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Chino.Collections;
using Chino.Objects;

namespace Chino.IO.Devices
{
    public class ConsoleDevice : Device, IConsoleDevice
    {
        private volatile bool _active = false;

        public event EventHandler? InputAvailable;

        protected ValueRingBuffer<ConsoleEvent> EventsBuffer;

        public override DeviceType DeviceType => DeviceType.Console;

        public ConsoleDevice(int capacity)
        {
            EventsBuffer = new ValueRingBuffer<ConsoleEvent>(capacity);
        }

        public int TryReadInput(Span<ConsoleEvent> buffer)
        {
            return EventsBuffer.TryRead(buffer);
        }

        protected override void Open(AccessMask grantedAccess)
        {
            _active = true;
            base.Open(grantedAccess);
        }

        protected void OnReceiveInputFromIsr(ConsoleEvent @event)
        {
            if (_active)
                EventsBuffer.TryWrite(MemoryMarshal.CreateReadOnlySpan(ref @event, 1));
        }
    }
}
