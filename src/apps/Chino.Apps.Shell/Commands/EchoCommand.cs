using System;
using System.Collections.Generic;
using System.Text;
using Chino.Memory;

namespace Chino.Apps.Shell.Commands
{
    class EchoCommand : ShellCommand
    {
        public override void Execute(string[] args)
        {
            for (int i = 1; i < args.Length; i++)
            {
                Console.Write(args[i]);
                if (i < args.Length - 1)
                    Console.Write(' ');
            }

            Console.WriteLine();
        }
    }
}
