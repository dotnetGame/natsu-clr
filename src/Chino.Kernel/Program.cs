using System;
using System.Diagnostics;
using System.Text;

namespace Chino.Kernel
{
    class Program
    {
        static void Main()
        {
            //var sb = new StringBuilder();
            //sb.AppendLine("Hello Chino OS!");
            //sb.AppendLine("Baka xiahuan!");

            //Action action = () => Debug.WriteLine(sb.ToString());
            //action();
            //Debug.WriteLine(sb.ToString());
            //byte i = 225;
            Debug.WriteLine("Hello Chino OS!");
            Debug.WriteLine("Used: " + Memory.MemoryManager.GetUsedMemorySize() + " Bytes");
            Debug.WriteLine("Free: " + Memory.MemoryManager.GetFreeMemorySize() + " Bytes");
        }
    }
}
