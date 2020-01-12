using System;
using System.Collections.Generic;
using System.Text;
using ThreadStart = System.Threading.ThreadStart;

namespace Chino.Threading
{
    public class Thread
    {
        private IScheduler _scheduler;
        private readonly ThreadStart _start;

        public Thread(ThreadStart start)
        {
            _start = start;
        }

        public void Start(object? arg)
        {
        }
    }
}
