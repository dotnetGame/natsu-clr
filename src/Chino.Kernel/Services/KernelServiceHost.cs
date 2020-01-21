using System;
using System.Collections.Generic;
using System.Text;
using Chino.Kernel;

namespace Chino.Services
{
    public class KernelServiceHost
    {
        public KernelServiceHost()
        {

        }

        public void Run()
        {
            int i = 0;
            while (true)
            {
                KernelServices.Scheduler.DelayCurrentThread(TimeSpan.FromSeconds(1));
                Terminal.Default.WriteLine("Tick " + i++.ToString());
            }
        }
    }
}
