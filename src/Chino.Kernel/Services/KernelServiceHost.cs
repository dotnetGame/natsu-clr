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
            float i = 0;
            while (true)
            {
                Terminal.Default.WriteLine("Tick " + i++);
                KernelServices.Scheduler.DelayCurrentThread(TimeSpan.FromSeconds(1));
            }
        }
    }
}
