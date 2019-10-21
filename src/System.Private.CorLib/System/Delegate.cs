using System;
using System.Collections.Generic;

namespace System
{
    public abstract class Delegate
    {
        // _target is the object we will invoke on
        internal object _target;

        // _methodPtr is a pointer to the method we will invoke
        // It could be a small thunk if this is a static or UM call
        internal IntPtr _methodPtr;

        // Protect the default constructor so you can't build a delegate
        internal Delegate()
        {
        }
    }
}
