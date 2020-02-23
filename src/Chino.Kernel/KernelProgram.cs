using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Chino.Chip;
using Chino.Services;
using Chino.Threading;

namespace Chino.Kernel
{
    class KernelProgram
    {
        static void KernelMain()
        {
            ChipControl.Default.Initialize();

            var systemThread = Scheduler.CreateThread(() =>
            {
                var host = new KernelServiceHost();

                var terminal = Terminal.Default;
                terminal.Foreground(TerminalColor.White)
                    .WriteLine("Hello Chino OS!");
                host.Run();
            });

            systemThread.SetDescription("System");
            systemThread.Start();
            Scheduler.StartCurrentScheduler();
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
        public static Terminal Default { get; } = new Terminal();

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
