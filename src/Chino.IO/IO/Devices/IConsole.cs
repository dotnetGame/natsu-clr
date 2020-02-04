using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Chino.IO.Devices
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
        public ConsoleEventType Type { get; private set; }

        [field: FieldOffset(4)]
        public KeyEvent Key { get; private set; }

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

        public static ConsoleEvent CreateKey(bool keyDown, char c)
        {
            return new ConsoleEvent
            {
                Type = ConsoleEventType.KeyEvent,
                Key = new KeyEvent(keyDown, c)
            };
        }
    }

    public interface IConsoleDevice
    {
        event EventHandler? InputAvailable;

        int TryReadInput(Span<ConsoleEvent> buffer);
    }
}
