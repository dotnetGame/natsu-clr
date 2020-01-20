// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;

namespace System.Diagnostics
{
    public partial class Stopwatch
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern long QueryPerformanceFrequency();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern long QueryPerformanceCounter();
    }
}