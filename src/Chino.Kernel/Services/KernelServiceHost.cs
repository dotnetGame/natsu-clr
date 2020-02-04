using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Chino.Chip;
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
            InitializeIOSystem();

            int i = 0;
            while (true)
            {
                Terminal.Default.WriteLine("Tick " + i++.ToString());
                Thread.Sleep(1000);
            }
        }

        private void InitializeIOSystem()
        {
            ChipControl.Default.InstallDrivers();
            ChipControl.Default.RegisterDeviceDescriptions();
        }
    }
}
