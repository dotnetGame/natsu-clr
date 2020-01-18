using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Threading
{
    public interface IScheduler
    {
        ulong TickCount { get; }
    }
}
