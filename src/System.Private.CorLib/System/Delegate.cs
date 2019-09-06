using System;
using System.Collections.Generic;

namespace System
{
    public abstract class Delegate
    {
        // _target is the object we will invoke on
        internal object _target;

        // MethodBase, either cached after first request or assigned from a DynamicMethod
        // For open delegates to collectible types, this may be a LoaderAllocator object
        internal object _methodBase;

        // _methodPtr is a pointer to the method we will invoke
        // It could be a small thunk if this is a static or UM call
        internal IntPtr _methodPtr;

        // In the case of a static method passed to a delegate, this field stores
        // whatever _methodPtr would have stored: and _methodPtr points to a
        // small thunk which removes the "this" pointer before going on
        // to _methodPtrAux.
        internal IntPtr _methodPtrAux;

        // Protect the default constructor so you can't build a delegate
        internal Delegate()
        {
        }
    }
}
