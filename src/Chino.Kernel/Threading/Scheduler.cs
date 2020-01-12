using System;
using System.Collections.Generic;

namespace Chino.Threading
{
    public class Scheduler
    {
        private readonly LinkedList<Thread> _runningThreads = new LinkedList<Thread>();
        private readonly LinkedList<Thread> _suspendedThreads = new LinkedList<Thread>();

        public Scheduler()
        {
        }
    }
}
