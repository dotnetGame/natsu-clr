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
            var usedMemory = MemoryManager.GetUsedMemorySize() / 1024f;
            var freeMemory = MemoryManager.GetFreeMemorySize() / 1024f;
            Console.WriteLine($"Memory\t\t Total\t {usedMemory + freeMemory:F2} K");
            Console.WriteLine($"\t\t  Used\t {usedMemory:F2} K");
            Console.WriteLine($"\t\t  Free\t {freeMemory:F2} K");
        }
    }
}
