using System;
using System.Diagnostics;
using System.Text;

namespace Chino.Kernel
{
    class Program
    {
        static void Main()
        {
            var sb = new StringBuilder();
            sb.AppendLine("Hello Chino OS!");
            sb.AppendLine("Baka xiahuan!");

            Debug.WriteLine(sb.ToString());
        }

        static string ToString(IComparable i)
        {
            return i.ToString();
        }
    }
}
