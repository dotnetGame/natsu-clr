// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
**
**
** Purpose: Identifies which SKU and version of the .NET
**   Framework that a particular library was compiled against.
**   Emitted by VS, and can help catch deployment problems.
**
===========================================================*/

using System;

namespace System.Runtime.Versioning
{
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = false, Inherited = false)]
    public sealed class TargetFrameworkAttribute : Attribute
    {
        // The target framework moniker that this assembly was compiled against.
        // Use the FrameworkName class to interpret target framework monikers.
        public string FrameworkName { get; }

        public string FrameworkDisplayName { get; set; }

        // The frameworkName parameter is intended to be the string form of a FrameworkName instance.
        public TargetFrameworkAttribute(string frameworkName)
        {
            FrameworkName = frameworkName;
        }
    }
}
