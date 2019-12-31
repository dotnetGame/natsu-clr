using Chino.Collections;
using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Devices.IO
{
    public abstract class Console : IConsole
    {
        public event EventHandler DataAvailable;

        private readonly ValueRingBuffer<ConsoleEvent> _eventsBuffer;

        public Console()
        {
            _eventsBuffer = new ValueRingBuffer<ConsoleEvent>(16);
        }
    }
}
