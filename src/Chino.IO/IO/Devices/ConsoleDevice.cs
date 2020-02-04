using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Chino.Collections;

namespace Chino.IO.Devices
{
    public class ConsoleDevice : Device, IConsoleDevice
    {
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

        protected void OnReceiveInputFromIsr(ConsoleEvent @event)
        {
            EventsBuffer.TryWrite(MemoryMarshal.CreateReadOnlySpan(ref @event, 1));
        }
    }
}
