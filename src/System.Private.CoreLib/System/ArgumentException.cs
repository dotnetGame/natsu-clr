// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*=============================================================================
**
**
**
** Purpose: Exception class for invalid arguments to a method.
**
**
=============================================================================*/

using System.Runtime.Serialization;

namespace System
{
    // The ArgumentException is thrown when an argument does not meet 
    // the contract of the method.  Ideally it should give a meaningful error
    // message describing what was wrong and which parameter is incorrect.
    [Serializable]
    public class ArgumentException : SystemException
    {
        private string _paramName;

        // Creates a new ArgumentException with its message 
        // string set to the empty string. 
        public ArgumentException()
            : base(nameof(ArgumentException))
        {
            HResult = HResults.COR_E_ARGUMENT;
        }

        // Creates a new ArgumentException with its message 
        // string set to message. 
        // 
        public ArgumentException(string message)
            : base(message)
        {
            HResult = HResults.COR_E_ARGUMENT;
        }

        public ArgumentException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = HResults.COR_E_ARGUMENT;
        }

        public ArgumentException(string message, string paramName, Exception innerException)
            : base(message, innerException)
        {
            _paramName = paramName;
            HResult = HResults.COR_E_ARGUMENT;
        }

        public ArgumentException(string message, string paramName)
            : base(message)
        {
            _paramName = paramName;
            HResult = HResults.COR_E_ARGUMENT;
        }

        public override string Message
        {
            get
            {
                return base.Message;
            }
        }

        public virtual string ParamName
        {
            get { return _paramName; }
        }
    }
}
