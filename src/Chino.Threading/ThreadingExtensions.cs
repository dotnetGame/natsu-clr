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

        public static uint GetId(this Accessor<Thread> thread)
        {
            return thread.Object.Id;
        }

        public static int GetExitCode(this Accessor<Thread> thread)
        {
            return thread.Object.ExitCode;
        }

        public static void Start(this Accessor<Thread> thread, object? arg = null)
        {
            thread.Object.Start(arg);
        }

        public static void WaitOne(this Accessor<Event> @event)
        {
            @event.Object.WaitOne(System.Threading.Timeout.InfiniteTimeSpan);
        }

        public static void WaitOne(this Accessor<Event> @event, TimeSpan timeout)
        {
            @event.Object.WaitOne(timeout);
        }

        public static void SetEvent(this Accessor<Event> @event)
        {
            @event.Object.SetEvent();
        }

        public static void ResetEvent(this Accessor<Event> @event)
        {
            @event.Object.ResetEvent();
        }
    }
}
