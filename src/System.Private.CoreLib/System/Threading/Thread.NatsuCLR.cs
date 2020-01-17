// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace System.Threading
{
    public partial class Thread
    {
        /// <summary>
        /// Suspends the current thread for timeout milliseconds. If timeout == 0,
        /// forces the thread to give up the remainder of its timeslice.  If timeout
        /// == Timeout.Infinite, no timeout will occur.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SleepInternal(int millisecondsTimeout);

        public static void Sleep(int millisecondsTimeout) => SleepInternal(millisecondsTimeout);

        /// <summary>
        /// Wait for a length of time proportional to 'iterations'.  Each iteration is should
        /// only take a few machine instructions.  Calling this API is preferable to coding
        /// a explicit busy loop because the hardware can be informed that it is busy waiting.
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SpinWaitInternal(int iterations);

        public static void SpinWait(int iterations) => SpinWaitInternal(iterations);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool YieldInternal();

        public static bool Yield() => YieldInternal();

        private static int s_optimalMaxSpinWaitsPerSpinIteration;

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetOptimalMaxSpinWaitsPerSpinIterationInternal();

        /// <summary>
        /// Max value to be passed into <see cref="SpinWait(int)"/> for optimal delaying. This value is normalized to be
        /// appropriate for the processor.
        /// </summary>
        internal static int OptimalMaxSpinWaitsPerSpinIteration
        {
            get
            {
                int optimalMaxSpinWaitsPerSpinIteration = s_optimalMaxSpinWaitsPerSpinIteration;
                return optimalMaxSpinWaitsPerSpinIteration != 0 ? optimalMaxSpinWaitsPerSpinIteration : CalculateOptimalMaxSpinWaitsPerSpinIteration();
            }
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static int CalculateOptimalMaxSpinWaitsPerSpinIteration()
        {
            // This is done lazily because the first call to the function below in the process triggers a measurement that
            // takes a nontrivial amount of time if the measurement has not already been done in the backgorund.
            // See Thread::InitializeYieldProcessorNormalized(), which describes and calculates this value.
            s_optimalMaxSpinWaitsPerSpinIteration = GetOptimalMaxSpinWaitsPerSpinIterationInternal();
            Debug.Assert(s_optimalMaxSpinWaitsPerSpinIteration > 0);
            return s_optimalMaxSpinWaitsPerSpinIteration;
        }
    }
}
