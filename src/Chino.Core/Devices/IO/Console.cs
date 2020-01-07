using Chino.Collections;
using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Devices.IO
{
    public abstract class Console : IConsole
    {
        public event EventHandler DataAvailable;

        protected ValueRingBuffer<ConsoleEvent> EventsBuffer;

        public Console()
        {
            EventsBuffer = new ValueRingBuffer<ConsoleEvent>(16);
        }
    }
}
