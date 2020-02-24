using System;
using System.Diagnostics;
using System.Threading;

namespace Chino.Apps.Shell
{
    class Program
    {
        static void Main(string[] args)
        {
            Debug.WriteLine("Hello Shell!");

            int i = 0;
            while (true)
            {
                Thread.Sleep(1000);
                Debug.WriteLine("Tick" + i++);
            }

            //Console.WriteLine("Hello Shell!");
        }
    }
}
