// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Globalization;

namespace System
{
    public struct Char
    {
        //
        // Member Variables
        //
        private short m_value; // Do not rename (binary serialization)

        //
        // Public Constants
        //
        // The maximum character value.
        public const char MaxValue = (char)0xFFFF;
        // The minimum character value.
        public const char MinValue = (char)0x00;

        // Return true for all characters below or equal U+00ff, which is ASCII + Latin-1 Supplement.
        private static bool IsLatin1(char ch)
        {
            return (ch <= '\x00ff');
        }

        // Return true for all characters below or equal U+007f, which is ASCII.
        private static bool IsAscii(char ch)
        {
            return (ch <= '\x007f');
        }

        private static bool IsWhiteSpaceLatin1(char c)
        {
            // There are characters which belong to UnicodeCategory.Control but are considered as white spaces.
            // We use code point comparisons for these characters here as a temporary fix.

            // U+0009 = <control> HORIZONTAL TAB
            // U+000a = <control> LINE FEED
            // U+000b = <control> VERTICAL TAB
            // U+000c = <contorl> FORM FEED
            // U+000d = <control> CARRIAGE RETURN
            // U+0085 = <control> NEXT LINE
            // U+00a0 = NO-BREAK SPACE
            return
                c == ' ' ||
                (uint)(c - '\x0009') <= ('\x000d' - '\x0009') || // (c >= '\x0009' && c <= '\x000d')
                c == '\x00a0' ||
                c == '\x0085';
        }

        /*===============================ISWHITESPACE===================================
        **A wrapper for char.  Returns a boolean indicating whether    **
        **character c is considered to be a whitespace character.                     **
        ==============================================================================*/
        // Determines whether a character is whitespace.
        public static bool IsWhiteSpace(char c)
        {
            if (IsLatin1(c))
            {
                return (IsWhiteSpaceLatin1(c));
            }
            return false;
        }
    }
}
