// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*=============================================================================
**
**
**
** Purpose: Exception class for null arguments to a method.
**
**
=============================================================================*/

using System.Runtime.Serialization;

namespace System
{
    // The ArgumentException is thrown when an argument 
    // is null when it shouldn't be.
    [Serializable]
    public class ArgumentNullException : ArgumentException
    {
        // Creates a new ArgumentNullException with its message 
        // string set to a default message explaining an argument was null.
        public ArgumentNullException()
             : base(nameof(ArgumentNullException))
        {
            // Use E_POINTER - COM used that for null pointers.  Description is "invalid pointer"
            HResult = HResults.E_POINTER;
        }

        public ArgumentNullException(string paramName)
            : base(paramName)
        {
            HResult = HResults.E_POINTER;
        }

        public ArgumentNullException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = HResults.E_POINTER;
        }

        public ArgumentNullException(string paramName, string message)
            : base(message, paramName)
        {
            HResult = HResults.E_POINTER;
        }
    }
}
