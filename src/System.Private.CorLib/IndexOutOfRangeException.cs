// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*=============================================================================
**
**
**
** Purpose: Exception class for invalid array indices.
**
**
=============================================================================*/

using System.Runtime.Serialization;

namespace System
{
    [Serializable]
    public sealed class IndexOutOfRangeException : SystemException
    {
        public IndexOutOfRangeException()
            //TODO: : base(SR.Arg_IndexOutOfRangeException)
            : base("Index out of range")
        {
            HResult = HResults.COR_E_INDEXOUTOFRANGE;
        }

        public IndexOutOfRangeException(string message)
            : base(message)
        {
            HResult = HResults.COR_E_INDEXOUTOFRANGE;
        }

        public IndexOutOfRangeException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = HResults.COR_E_INDEXOUTOFRANGE;
        }
    }
}
