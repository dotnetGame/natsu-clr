// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;

// The code below includes partial support for float/double and
// pointer sized enums.
//
// The type loader does not prohibit such enums, and older versions of
// the ECMA spec include them as possible enum types.
//
// However there are many things broken throughout the stack for
// float/double/intptr/uintptr enums. There was a conscious decision
// made to not fix the whole stack to work well for them because of
// the right behavior is often unclear, and it is hard to test and
// very low value because of such enums cannot be expressed in C#.

namespace System
{
    public abstract class Enum : ValueType
    {
    }
}
