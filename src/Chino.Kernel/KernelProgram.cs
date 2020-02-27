using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Chino.Chip;
using Chino.Services;
using Chino.Threading;

namespace Chino.Kernel
{
    class KernelProgram
    {
        static void KernelMain()
        {
            ChipControl.Default.Initialize();

            var systemThread = Scheduler.CreateThread(() =>
            {
                var host = new KernelServiceHost();
                host.Run();
            });

            systemThread.SetDescription("System");
            systemThread.Start();
            Scheduler.StartCurrentScheduler();
        }
    }
}
