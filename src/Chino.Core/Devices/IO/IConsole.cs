using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Chino.Devices.IO
{
    public enum ConsoleEventType
    {
        Invalid = 0,
        KeyEvent = 1
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct ConsoleEvent
    {
        [field: FieldOffset(0)]
        public ConsoleEventType Type { get; }

        [field: FieldOffset(4)]
        public KeyEvent Key { get; }

        public struct KeyEvent
        {
            public bool KeyDown { get; }

            public char Char { get; }

            public KeyEvent(bool keyDown, char c)
            {
                KeyDown = keyDown;
                Char = c;
            }
        }
    }

    public interface IConsole
    {
        event EventHandler DataAvailable;
    }
}
