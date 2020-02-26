using System;
using System.Diagnostics;
using System.Threading;

namespace Chino.Apps.Shell
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("Hello Shell!");
            Console.ForegroundColor = ConsoleColor.White;

            int i = 0;
            while (true)
            {
                Thread.Sleep(1000);
                Console.WriteLine("Tick" + i++);
            }
        }
    }
}
