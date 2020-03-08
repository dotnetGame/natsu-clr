using System;
using System.Collections.Generic;
using System.Text;
using Chino.Memory;

namespace Chino.Apps.Shell.Commands
{
    class FreeCommand : ShellCommand
    {
        public override void Execute(CommandContext context, string[] args)
        {
            var usedMemory = MemoryManager.GetUsedMemorySize();
            var freeMemory = MemoryManager.GetFreeMemorySize();
            Console.WriteLine($"\ttotal\t\tused\t\tfree");
            Console.WriteLine($"Mem:\t{usedMemory + freeMemory}\t\t{usedMemory}\t\t{freeMemory}");
        }
    }
}
