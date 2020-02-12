using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;
using Chino.Threading;

namespace Chino
{
    public static class ThreadingExtensions
    {
        public static string? GetDescription(this IAccessor<Thread> thread)
        {
            return thread.Object.Description;
        }

        public static void SetDescription(this IAccessor<Thread> thread, string? value)
        {
            thread.Object.Description = value;
        }

        public static uint GetId(this IAccessor<Thread> thread)
        {
            return thread.Object.Id;
        }

        public static int GetExitCode(this IAccessor<Thread> thread)
        {
            return thread.Object.ExitCode;
        }

        public static void Start(this IAccessor<Thread> thread, object? arg = null)
        {
            thread.Object.Start(arg);
        }

        public static void WaitOne(this IAccessor<Event> @event)
        {
            @event.Object.WaitOne(System.Threading.Timeout.InfiniteTimeSpan);
        }

        public static void WaitOne(this IAccessor<Event> @event, TimeSpan timeout)
        {
            @event.Object.WaitOne(timeout);
        }

        public static void SetEvent(this IAccessor<Event> @event)
        {
            @event.Object.SetEvent();
        }

        public static void ResetEvent(this IAccessor<Event> @event)
        {
            @event.Object.ResetEvent();
        }
    }
}
