using System;
using System.Diagnostics;
using System.Threading;
using Chino.Memory;

namespace Chino.Apps.Shell
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.ForegroundColor = ConsoleColor.Green;

            // Print logo
            Console.WriteLine(@"
 _____ _     _             
/  __ \ |   (_)            
| /  \/ |__  _ _ __   ___  
| |   | '_ \| | '_ \ / _ \ 
| \__/\ | | | | | | | (_) |
 \____/_| |_|_|_| |_|\___/ 
                           ");
            // Print memory status
            var usedMemory = MemoryManager.GetUsedMemorySize() / 1024f;
            var freeMemory = MemoryManager.GetFreeMemorySize() / 1024f;
            Console.WriteLine($"Memory\t\t Total\t {usedMemory + freeMemory:F2} K");
            Console.WriteLine($"\t\t  Used\t {usedMemory:F2} K");
            Console.WriteLine($"\t\t  Free\t {freeMemory:F2} K");
            Console.WriteLine();
            Console.ForegroundColor = ConsoleColor.White;

            var interpreter = new CommandInterpreter();
            interpreter.Run();
        }
    }
}
