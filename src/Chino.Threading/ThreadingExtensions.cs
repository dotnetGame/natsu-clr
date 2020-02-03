using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;
using Chino.Threading;

namespace Chino
{
    public static class ThreadingExtensions
    {
        public static string? GetDescription(this Accessor<Thread> thread)
        {
            return thread.Object.Description;
        }

        public static void SetDescription(this Accessor<Thread> thread, string? value)
        {
            thread.Object.Description = value;
        }

        public static int GetExitCode(this Accessor<Thread> thread)
        {
            return thread.Object.ExitCode;
        }

        public static void Start(this Accessor<Thread> thread, object? arg = null)
        {
            thread.Object.Start(arg);
        }
    }
}
