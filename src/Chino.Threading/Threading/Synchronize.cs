using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;

namespace Chino.Threading
{
    public static class Synchronize
    {
        public static Accessor<Event> Create(string? name = null, bool initialState = false, bool autoReset = true)
        {
            return ObjectManager.CreateObject(new Event(initialState, autoReset), AccessMask.GenericAll, default);
        }
    }
}
