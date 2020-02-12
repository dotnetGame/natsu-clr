// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Reflection;
using System.Globalization;
using System.Threading;
using System.Runtime.CompilerServices;

namespace System
{
    /// <summary>
    /// Activator contains the Activation (CreateInstance/New) methods for late bound support.
    /// </summary>
    public static class Activator
    {
        internal const BindingFlags ConstructorDefault = BindingFlags.Instance | BindingFlags.Public | BindingFlags.CreateInstance;

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern T CreateInstance<T>();
    }
}
