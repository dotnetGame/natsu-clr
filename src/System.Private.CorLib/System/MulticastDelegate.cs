// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Reflection;
using System.Runtime.Serialization;
using System.Diagnostics;
using Internal.Runtime.CompilerServices;

namespace System
{
    public abstract class MulticastDelegate : Delegate
    {
        // This is set under 3 circumstances
        // 1. Multicast delegate
        // 2. Secure/Wrapper delegate
        // 3. Inner delegate of secure delegate where the secure delegate security context is a collectible method
        private object _invocationList;
        private IntPtr _invocationCount;

        internal bool IsUnmanagedFunctionPtr()
        {
            return (_invocationCount == (IntPtr)(-1));
        }

        internal bool InvocationListLogicallyNull()
        {
            return (_invocationList == null);
        }
    }
}
