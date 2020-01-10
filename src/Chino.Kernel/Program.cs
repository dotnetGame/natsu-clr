using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Chino.Kernel
{
    class Program
    {
        static void Main()
        {
            ChipControl.Initialize();

            var terminal = new Terminal();

            terminal.Foreground(TerminalColor.White)
                .WriteLine("Hello Chino OS!");

            terminal.Foreground(TerminalColor.White)
                .Write("Used: ").Foreground(TerminalColor.Green).Write(Memory.MemoryManager.GetUsedMemorySize().ToString())
                .Foreground(TerminalColor.White).WriteLine(" Bytes");
            terminal.Foreground(TerminalColor.White)
                .Write("Free: ").Foreground(TerminalColor.Green).Write(Memory.MemoryManager.GetFreeMemorySize().ToString())
                .Foreground(TerminalColor.White).WriteLine(" Bytes");
            terminal.WriteLine().Reset();

            var l = new List<int> { 1, 2, 3 };
            foreach (var item in l)
                terminal.Write(item + ", ");
            l.Clear();
            terminal.WriteLine();
            l.Add(4);
            foreach (var item in l)
                terminal.Write(item + ", ");

            terminal.WriteLine();
            terminal.Ready();
        }
    }

    enum TerminalColor
    {
        Black,
        Red,
        Green,
        White
    }

    class Terminal
    {
        public Terminal Write(string message)
        {
            Debug.Write(message);
            return this;
        }

        public Terminal WriteLine(string message)
        {
            Debug.WriteLine(message);
            return this;
        }

        public Terminal WriteLine()
        {
            Debug.WriteLine(string.Empty);
            return this;
        }

        public Terminal Reset()
        {
            Debug.Write("\u001b[0m");
            return this;
        }

        public Terminal Foreground(TerminalColor color)
        {
            string seq = color switch
            {
                TerminalColor.Black => "\u001b[30m",
                TerminalColor.Red => "\u001b[31m",
                TerminalColor.Green => "\u001b[32m",
                TerminalColor.White => "\u001b[37m",
                _ => string.Empty
            };

            Debug.Write(seq);
            return this;
        }

        public Terminal Ready()
        {
            Debug.Write("$ ");
            return this;
        }
    }
}
