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

        //
        // Internal Constants are real consts for performance.
        //

        // The internal string representation of true.
        // 
        internal const string TrueLiteral = "True";

        // The internal string representation of false.
        // 
        internal const string FalseLiteral = "False";


        //
        // Public Constants
        //

        // The public string representation of true.
        // 
        public static readonly string TrueString = TrueLiteral;

        // The public string representation of false.
        // 
        public static readonly string FalseString = FalseLiteral;

        //
        // Overriden Instance Methods
        //
        /*=================================GetHashCode==================================
        **Args:  None
        **Returns: 1 or 0 depending on whether this instance represents true or false.
        **Exceptions: None
        **Overriden From: Value
        ==============================================================================*/
        // Provides a hash code for this instance.
        public override int GetHashCode()
        {
            return (m_value) ? True : False;
        }

        /*===================================ToString===================================
        **Args: None
        **Returns:  "True" or "False" depending on the state of the boolean.
        **Exceptions: None.
        ==============================================================================*/
        // Converts the boolean value of this instance to a String.
        public override string ToString()
        {
            if (false == m_value)
            {
                return FalseLiteral;
            }
            return TrueLiteral;
        }
    }
}
