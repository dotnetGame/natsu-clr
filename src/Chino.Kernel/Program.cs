using System;
using System.Diagnostics;

namespace Chino.Kernel
{
    class Program
    {
        static void Main()
        {
            bool b = true;
            for (int i = 0; i < 3; i++)
            {
                Debug.WriteLine("Hello Chino OS! " + b.ToString());
                b = !b;
            }
        }

        static string ToString(IComparable i)
        {
            return i.ToString();
        }
    }
}
