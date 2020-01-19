using System;
using System.Collections.Generic;
using System.Text;
using Chino.Threading;

namespace Chino
{
    public class KernelServices
    {
        public static Scheduler Scheduler { get; private set; }

        internal static void Initialize()
        {
            Scheduler = new Scheduler();
        }
    }
}
