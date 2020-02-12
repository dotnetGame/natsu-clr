using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using Chino.Chip;
using Chino.Kernel;
using Chino.Threading;

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

            var userThread = Scheduler.CreateThread(UserAppMain);
            userThread.SetDescription("UserAppMain");
            userThread.Start();

            //int i = 0;
            while (true)
            {
                //Terminal.Default.WriteLine("Tick " + i++.ToString());
                System.Threading.Thread.Sleep(1000);
            }
        }

        private void InitializeIOSystem()
        {
            ChipControl.Default.InstallDrivers();
            ChipControl.Default.RegisterDeviceDescriptions();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void UserAppMain();
    }
}
