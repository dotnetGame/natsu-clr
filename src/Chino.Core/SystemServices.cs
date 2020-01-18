using Chino.Threading;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;

namespace Chino
{
    public static class SystemServices
    {
        private static ISystemServices _services;
        public static IScheduler Scheduler => _services.Scheduler;

        static SystemServices()
        {
            Initialize();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Initialize();
    }

    internal interface ISystemServices
    {
        IScheduler Scheduler { get; }
    }
}
