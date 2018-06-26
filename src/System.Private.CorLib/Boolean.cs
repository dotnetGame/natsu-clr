// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
**
**
** Purpose: The boolean class serves as a wrapper for the primitive
** type boolean.
**
** 
===========================================================*/

using System.Runtime.CompilerServices;

namespace System
{
    public struct Boolean
    {
        //
        // Member Variables
        //
        private bool m_value; // Do not rename (binary serialization)

        // The true value.
        //
        internal const int True = 1;

        // The false value.
        //
        internal const int False = 0;
    }
}
