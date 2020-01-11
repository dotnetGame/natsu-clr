using System;
using System.Collections.Generic;

namespace Chino.Threading
{
    public class ThreadManager
    {
        public static ThreadManager Default { get; } = new ThreadManager();

        private readonly LinkedList<Thread> _runningThreads = new LinkedList<Thread>();
        private readonly LinkedList<Thread> _readyThreads = new LinkedList<Thread>();
        private readonly LinkedList<Thread> _suspendedThreads = new LinkedList<Thread>();
    }
}
