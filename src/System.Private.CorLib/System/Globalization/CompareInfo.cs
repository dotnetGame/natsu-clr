// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

////////////////////////////////////////////////////////////////////////////
//
//
//
//  Purpose:  This class implements a set of methods for comparing
//            strings.
//
//
////////////////////////////////////////////////////////////////////////////

using System.Reflection;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Buffers;
using Internal.Runtime.CompilerServices;
using System.Runtime.CompilerServices;

namespace System.Globalization
{
    [Flags]
    public enum CompareOptions
    {
        None = 0x00000000,
        IgnoreCase = 0x00000001,
        IgnoreNonSpace = 0x00000002,
        IgnoreSymbols = 0x00000004,
        IgnoreKanaType = 0x00000008,   // ignore kanatype
        IgnoreWidth = 0x00000010,   // ignore width
        OrdinalIgnoreCase = 0x10000000,   // This flag can not be used with other flags.
        StringSort = 0x20000000,   // use string sort method
        Ordinal = 0x40000000,   // This flag can not be used with other flags.
    }

    [Serializable]
    [System.Runtime.CompilerServices.TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public partial class CompareInfo
    {
        // Mask used to check if IndexOf()/LastIndexOf()/IsPrefix()/IsPostfix() has the right flags.
        private const CompareOptions ValidIndexMaskOffFlags =
            ~(CompareOptions.IgnoreCase | CompareOptions.IgnoreSymbols | CompareOptions.IgnoreNonSpace |
              CompareOptions.IgnoreWidth | CompareOptions.IgnoreKanaType);

        // Mask used to check if Compare() has the right flags.
        private const CompareOptions ValidCompareMaskOffFlags =
            ~(CompareOptions.IgnoreCase | CompareOptions.IgnoreSymbols | CompareOptions.IgnoreNonSpace |
              CompareOptions.IgnoreWidth | CompareOptions.IgnoreKanaType | CompareOptions.StringSort);

        // Mask used to check if GetHashCodeOfString() has the right flags.
        private const CompareOptions ValidHashCodeOfStringMaskOffFlags =
            ~(CompareOptions.IgnoreCase | CompareOptions.IgnoreSymbols | CompareOptions.IgnoreNonSpace |
              CompareOptions.IgnoreWidth | CompareOptions.IgnoreKanaType);

        // Mask used to check if we have the right flags.
        private const CompareOptions ValidSortkeyCtorMaskOffFlags =
            ~(CompareOptions.IgnoreCase | CompareOptions.IgnoreSymbols | CompareOptions.IgnoreNonSpace |
              CompareOptions.IgnoreWidth | CompareOptions.IgnoreKanaType | CompareOptions.StringSort);

        // Cache the invariant CompareInfo
        internal static readonly CompareInfo Invariant = CultureInfo.InvariantCulture.CompareInfo;

        //
        // CompareInfos have an interesting identity.  They are attached to the locale that created them,
        // ie: en-US would have an en-US sort.  For haw-US (custom), then we serialize it as haw-US.
        // The interesting part is that since haw-US doesn't have its own sort, it has to point at another
        // locale, which is what SCOMPAREINFO does.
        [OptionalField(VersionAdded = 2)]
        private string m_name;  // The name used to construct this CompareInfo. Do not rename (binary serialization)

        [NonSerialized]
        private string _sortName; // The name that defines our behavior

        [OptionalField(VersionAdded = 3)]
        private SortVersion m_SortVersion; // Do not rename (binary serialization)

        // _invariantMode is defined for the perf reason as accessing the instance field is faster than access the static property GlobalizationMode.Invariant
        [NonSerialized]
        private readonly bool _invariantMode = GlobalizationMode.Invariant;

        private int culture; // Do not rename (binary serialization). The fields sole purpose is to support Desktop serialization.

        internal CompareInfo(CultureInfo culture)
        {
            m_name = culture._name;
        }

        ///////////////////////////----- Name -----/////////////////////////////////
        //
        //  Returns the name of the culture (well actually, of the sort).
        //  Very important for providing a non-LCID way of identifying
        //  what the sort is.
        //
        //  Note that this name isn't dereferenced in case the CompareInfo is a different locale
        //  which is consistent with the behaviors of earlier versions.  (so if you ask for a sort
        //  and the locale's changed behavior, then you'll get changed behavior, which is like
        //  what happens for a version update)
        //
        ////////////////////////////////////////////////////////////////////////

        public virtual string Name
        {
            get
            {
                Debug.Assert(m_name != null, "CompareInfo.Name Expected _name to be set");
                if (m_name == "zh-CHT" || m_name == "zh-CHS")
                {
                    return m_name;
                }

                return _sortName;
            }
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  Compare
        //
        //  Compares the two strings with the given options.  Returns 0 if the
        //  two strings are equal, a number less than 0 if string1 is less
        //  than string2, and a number greater than 0 if string1 is greater
        //  than string2.
        //
        ////////////////////////////////////////////////////////////////////////

        public virtual int Compare(string string1, string string2)
        {
            return Compare(string1, string2, CompareOptions.None);
        }

        public virtual int Compare(string string1, string string2, CompareOptions options)
        {
            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return string.Compare(string1, string2, StringComparison.OrdinalIgnoreCase);
            }

            // Verify the options before we do any real comparison.
            if ((options & CompareOptions.Ordinal) != 0)
            {
                if (options != CompareOptions.Ordinal)
                {
                    throw new ArgumentException(SR.Argument_CompareOptionOrdinal, nameof(options));
                }

                return string.CompareOrdinal(string1, string2);
            }

            if ((options & ValidCompareMaskOffFlags) != 0)
            {
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));
            }

            //Our paradigm is that null sorts less than any other string and
            //that two nulls sort as equal.
            if (string1 == null)
            {
                if (string2 == null)
                {
                    return (0);     // Equal
                }
                return (-1);    // null < non-null
            }
            if (string2 == null)
            {
                return (1);     // non-null > null
            }

            if ((options & CompareOptions.IgnoreCase) != 0)
                return CompareOrdinalIgnoreCase(string1, string2);

            return string.CompareOrdinal(string1, string2);
        }

        // TODO https://github.com/dotnet/coreclr/issues/13827:
        // This method shouldn't be necessary, as we should be able to just use the overload
        // that takes two spans.  But due to this issue, that's adding significant overhead.
        internal int Compare(ReadOnlySpan<char> string1, string string2, CompareOptions options)
        {
            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return CompareOrdinalIgnoreCase(string1, string2.AsSpan());
            }

            // Verify the options before we do any real comparison.
            if ((options & CompareOptions.Ordinal) != 0)
            {
                if (options != CompareOptions.Ordinal)
                {
                    throw new ArgumentException(SR.Argument_CompareOptionOrdinal, nameof(options));
                }

                return string.CompareOrdinal(string1, string2.AsSpan());
            }

            if ((options & ValidCompareMaskOffFlags) != 0)
            {
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));
            }

            // null sorts less than any other string.
            if (string2 == null)
            {
                return 1;
            }

            return (options & CompareOptions.IgnoreCase) != 0 ?
                CompareOrdinalIgnoreCase(string1, string2.AsSpan()) :
                string.CompareOrdinal(string1, string2.AsSpan());
        }

        internal int CompareOptionNone(ReadOnlySpan<char> string1, ReadOnlySpan<char> string2)
        {
            // Check for empty span or span from a null string
            if (string1.Length == 0 || string2.Length == 0)
                return string1.Length - string2.Length;

            return string.CompareOrdinal(string1, string2);
        }

        internal int CompareOptionIgnoreCase(ReadOnlySpan<char> string1, ReadOnlySpan<char> string2)
        {
            // Check for empty span or span from a null string
            if (string1.Length == 0 || string2.Length == 0)
                return string1.Length - string2.Length;

            return CompareOrdinalIgnoreCase(string1, string2);
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  Compare
        //
        //  Compares the specified regions of the two strings with the given
        //  options.
        //  Returns 0 if the two strings are equal, a number less than 0 if
        //  string1 is less than string2, and a number greater than 0 if
        //  string1 is greater than string2.
        //
        ////////////////////////////////////////////////////////////////////////


        public virtual int Compare(string string1, int offset1, int length1, string string2, int offset2, int length2)
        {
            return Compare(string1, offset1, length1, string2, offset2, length2, 0);
        }


        public virtual int Compare(string string1, int offset1, string string2, int offset2, CompareOptions options)
        {
            return Compare(string1, offset1, string1 == null ? 0 : string1.Length - offset1,
                           string2, offset2, string2 == null ? 0 : string2.Length - offset2, options);
        }


        public virtual int Compare(string string1, int offset1, string string2, int offset2)
        {
            return Compare(string1, offset1, string2, offset2, 0);
        }


        public virtual int Compare(string string1, int offset1, int length1, string string2, int offset2, int length2, CompareOptions options)
        {
            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                int result = string.Compare(string1, offset1, string2, offset2, length1 < length2 ? length1 : length2, StringComparison.OrdinalIgnoreCase);
                if ((length1 != length2) && result == 0)
                    return (length1 > length2 ? 1 : -1);
                return (result);
            }

            // Verify inputs
            if (length1 < 0 || length2 < 0)
            {
                throw new ArgumentOutOfRangeException((length1 < 0) ? nameof(length1) : nameof(length2), SR.ArgumentOutOfRange_NeedPosNum);
            }
            if (offset1 < 0 || offset2 < 0)
            {
                throw new ArgumentOutOfRangeException((offset1 < 0) ? nameof(offset1) : nameof(offset2), SR.ArgumentOutOfRange_NeedPosNum);
            }
            if (offset1 > (string1 == null ? 0 : string1.Length) - length1)
            {
                throw new ArgumentOutOfRangeException(nameof(string1), SR.ArgumentOutOfRange_OffsetLength);
            }
            if (offset2 > (string2 == null ? 0 : string2.Length) - length2)
            {
                throw new ArgumentOutOfRangeException(nameof(string2), SR.ArgumentOutOfRange_OffsetLength);
            }
            if ((options & CompareOptions.Ordinal) != 0)
            {
                if (options != CompareOptions.Ordinal)
                {
                    throw new ArgumentException(SR.Argument_CompareOptionOrdinal,
                                                nameof(options));
                }
            }
            else if ((options & ValidCompareMaskOffFlags) != 0)
            {
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));
            }

            //
            // Check for the null case.
            //
            if (string1 == null)
            {
                if (string2 == null)
                {
                    return (0);
                }
                return (-1);
            }
            if (string2 == null)
            {
                return (1);
            }

            ReadOnlySpan<char> span1 = string1.AsSpan(offset1, length1);
            ReadOnlySpan<char> span2 = string2.AsSpan(offset2, length2);

            if (options == CompareOptions.Ordinal)
            {
                return string.CompareOrdinal(span1, span2);
            }

            if ((options & CompareOptions.IgnoreCase) != 0)
                return CompareOrdinalIgnoreCase(span1, span2);

            return string.CompareOrdinal(span1, span2);
        }

        //
        // CompareOrdinalIgnoreCase compare two string ordinally with ignoring the case.
        // it assumes the strings are Ascii string till we hit non Ascii character in strA or strB and then we continue the comparison by
        // calling the OS.
        //
        internal static int CompareOrdinalIgnoreCase(string strA, int indexA, int lengthA, string strB, int indexB, int lengthB)
        {
            Debug.Assert(indexA + lengthA <= strA.Length);
            Debug.Assert(indexB + lengthB <= strB.Length);
            return CompareOrdinalIgnoreCase(
                ref Unsafe.Add(ref strA.GetRawStringData(), indexA),
                lengthA,
                ref Unsafe.Add(ref strB.GetRawStringData(), indexB),
                lengthB);
        }

        internal static int CompareOrdinalIgnoreCase(ReadOnlySpan<char> strA, ReadOnlySpan<char> strB)
        {
            return CompareOrdinalIgnoreCase(ref MemoryMarshal.GetReference(strA), strA.Length, ref MemoryMarshal.GetReference(strB), strB.Length);
        }

        internal static int CompareOrdinalIgnoreCase(string strA, string strB)
        {
            return CompareOrdinalIgnoreCase(ref strA.GetRawStringData(), strA.Length, ref strB.GetRawStringData(), strB.Length);
        }

        internal static int CompareOrdinalIgnoreCase(ref char strA, int lengthA, ref char strB, int lengthB)
        {
            int length = Math.Min(lengthA, lengthB);
            int range = length;

            ref char charA = ref strA;
            ref char charB = ref strB;

            // in InvariantMode we support all range and not only the ascii characters.
            char maxChar = (GlobalizationMode.Invariant ? (char)0xFFFF : (char)0x7F);

            while (length != 0 && charA <= maxChar && charB <= maxChar)
            {
                // Ordinal equals or lowercase equals if the result ends up in the a-z range 
                if (charA == charB ||
                    ((charA | 0x20) == (charB | 0x20) &&
                        (uint)((charA | 0x20) - 'a') <= (uint)('z' - 'a')))
                {
                    length--;
                    charA = ref Unsafe.Add(ref charA, 1);
                    charB = ref Unsafe.Add(ref charB, 1);
                }
                else
                {
                    int currentA = charA;
                    int currentB = charB;

                    // Uppercase both chars if needed
                    if ((uint)(charA - 'a') <= 'z' - 'a')
                        currentA -= 0x20;
                    if ((uint)(charB - 'a') <= 'z' - 'a')
                        currentB -= 0x20;

                    // Return the (case-insensitive) difference between them.
                    return currentA - currentB;
                }
            }

            if (length == 0)
                return lengthA - lengthB;

            Debug.Assert(!GlobalizationMode.Invariant);
            throw new NotImplementedException();
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  IsPrefix
        //
        //  Determines whether prefix is a prefix of string.  If prefix equals
        //  string.Empty, true is returned.
        //
        ////////////////////////////////////////////////////////////////////////
        public virtual bool IsPrefix(string source, string prefix, CompareOptions options)
        {
            if (source == null || prefix == null)
            {
                throw new ArgumentNullException((source == null ? nameof(source) : nameof(prefix)),
                    SR.ArgumentNull_String);
            }

            if (prefix.Length == 0)
            {
                return (true);
            }

            if (source.Length == 0)
            {
                return false;
            }

            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return source.StartsWith(prefix, StringComparison.OrdinalIgnoreCase);
            }

            if (options == CompareOptions.Ordinal)
            {
                return source.StartsWith(prefix, StringComparison.Ordinal);
            }

            if ((options & ValidIndexMaskOffFlags) != 0)
            {
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));
            }

            return source.StartsWith(prefix, (options & CompareOptions.IgnoreCase) != 0 ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal);
        }

        internal bool IsPrefix(ReadOnlySpan<char> source, ReadOnlySpan<char> prefix, CompareOptions options)
        {
            Debug.Assert(prefix.Length != 0);
            Debug.Assert(source.Length != 0);
            Debug.Assert((options & ValidIndexMaskOffFlags) == 0);
            Debug.Assert(!_invariantMode);
            Debug.Assert((options & (CompareOptions.Ordinal | CompareOptions.OrdinalIgnoreCase)) == 0);

            return source.StartsWith(prefix);
        }

        public virtual bool IsPrefix(string source, string prefix)
        {
            return (IsPrefix(source, prefix, 0));
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  IsSuffix
        //
        //  Determines whether suffix is a suffix of string.  If suffix equals
        //  string.Empty, true is returned.
        //
        ////////////////////////////////////////////////////////////////////////
        public virtual bool IsSuffix(string source, string suffix, CompareOptions options)
        {
            if (source == null || suffix == null)
            {
                throw new ArgumentNullException((source == null ? nameof(source) : nameof(suffix)),
                    SR.ArgumentNull_String);
            }

            if (suffix.Length == 0)
            {
                return (true);
            }

            if (source.Length == 0)
            {
                return false;
            }

            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return source.EndsWith(suffix, StringComparison.OrdinalIgnoreCase);
            }

            if (options == CompareOptions.Ordinal)
            {
                return source.EndsWith(suffix, StringComparison.Ordinal);
            }

            if ((options & ValidIndexMaskOffFlags) != 0)
            {
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));
            }
            return source.EndsWith(suffix, (options & CompareOptions.IgnoreCase) != 0 ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal);
        }


        public virtual bool IsSuffix(string source, string suffix)
        {
            return (IsSuffix(source, suffix, 0));
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  IndexOf
        //
        //  Returns the first index where value is found in string.  The
        //  search starts from startIndex and ends at endIndex.  Returns -1 if
        //  the specified value is not found.  If value equals string.Empty,
        //  startIndex is returned.  Throws IndexOutOfRange if startIndex or
        //  endIndex is less than zero or greater than the length of string.
        //  Throws ArgumentException if value is null.
        //
        ////////////////////////////////////////////////////////////////////////


        public virtual int IndexOf(string source, char value)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, 0, source.Length, CompareOptions.None);
        }


        public virtual int IndexOf(string source, string value)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, 0, source.Length, CompareOptions.None);
        }


        public virtual int IndexOf(string source, char value, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, 0, source.Length, options);
        }


        public virtual int IndexOf(string source, string value, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, 0, source.Length, options);
        }

        public virtual int IndexOf(string source, char value, int startIndex)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, startIndex, source.Length - startIndex, CompareOptions.None);
        }

        public virtual int IndexOf(string source, string value, int startIndex)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, startIndex, source.Length - startIndex, CompareOptions.None);
        }

        public virtual int IndexOf(string source, char value, int startIndex, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, startIndex, source.Length - startIndex, options);
        }


        public virtual int IndexOf(string source, string value, int startIndex, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            return IndexOf(source, value, startIndex, source.Length - startIndex, options);
        }


        public virtual int IndexOf(string source, char value, int startIndex, int count)
        {
            return IndexOf(source, value, startIndex, count, CompareOptions.None);
        }


        public virtual int IndexOf(string source, string value, int startIndex, int count)
        {
            return IndexOf(source, value, startIndex, count, CompareOptions.None);
        }

        public unsafe virtual int IndexOf(string source, char value, int startIndex, int count, CompareOptions options)
        {
            // Validate inputs
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            if (startIndex < 0 || startIndex > source.Length)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);

            if (count < 0 || startIndex > source.Length - count)
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_Count);

            if (source.Length == 0)
            {
                return -1;
            }

            // Validate CompareOptions
            // Ordinal can't be selected with other flags
            if ((options & ValidIndexMaskOffFlags) != 0 && (options != CompareOptions.Ordinal))
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));

            return IndexOfOrdinal(source, new string(value, 1), startIndex, count, ignoreCase: (options & (CompareOptions.IgnoreCase | CompareOptions.OrdinalIgnoreCase)) != 0);
        }

        public unsafe virtual int IndexOf(string source, string value, int startIndex, int count, CompareOptions options)
        {
            // Validate inputs
            if (source == null)
                throw new ArgumentNullException(nameof(source));
            if (value == null)
                throw new ArgumentNullException(nameof(value));

            if (startIndex > source.Length)
            {
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);
            }

            // In Everett we used to return -1 for empty string even if startIndex is negative number so we keeping same behavior here.
            // We return 0 if both source and value are empty strings for Everett compatibility too.
            if (source.Length == 0)
            {
                if (value.Length == 0)
                {
                    return 0;
                }
                return -1;
            }

            if (startIndex < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);
            }

            if (count < 0 || startIndex > source.Length - count)
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_Count);

            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return IndexOfOrdinal(source, value, startIndex, count, ignoreCase: true);
            }

            // Validate CompareOptions
            // Ordinal can't be selected with other flags
            if ((options & ValidIndexMaskOffFlags) != 0 && (options != CompareOptions.Ordinal))
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));

            return IndexOfOrdinal(source, value, startIndex, count, ignoreCase: (options & (CompareOptions.IgnoreCase | CompareOptions.OrdinalIgnoreCase)) != 0);
        }

        // The following IndexOf overload is mainly used by String.Replace. This overload assumes the parameters are already validated
        // and the caller is passing a valid matchLengthPtr pointer.
        internal unsafe int IndexOf(string source, string value, int startIndex, int count, CompareOptions options, int* matchLengthPtr)
        {
            Debug.Assert(source != null);
            Debug.Assert(value != null);
            Debug.Assert(startIndex >= 0);
            Debug.Assert(matchLengthPtr != null);
            *matchLengthPtr = 0;

            if (source.Length == 0)
            {
                if (value.Length == 0)
                {
                    return 0;
                }
                return -1;
            }

            if (startIndex >= source.Length)
            {
                return -1;
            }

            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                int res = IndexOfOrdinal(source, value, startIndex, count, ignoreCase: true);
                if (res >= 0)
                {
                    *matchLengthPtr = value.Length;
                }
                return res;
            }

            {
                int res = IndexOfOrdinal(source, value, startIndex, count, ignoreCase: (options & (CompareOptions.IgnoreCase | CompareOptions.OrdinalIgnoreCase)) != 0);
                if (res >= 0)
                {
                    *matchLengthPtr = value.Length;
                }
                return res;
            }
        }

        internal int IndexOfOrdinal(string source, string value, int startIndex, int count, bool ignoreCase)
        {
            return InvariantIndexOf(source, value, startIndex, count, ignoreCase);
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  LastIndexOf
        //
        //  Returns the last index where value is found in string.  The
        //  search starts from startIndex and ends at endIndex.  Returns -1 if
        //  the specified value is not found.  If value equals string.Empty,
        //  endIndex is returned.  Throws IndexOutOfRange if startIndex or
        //  endIndex is less than zero or greater than the length of string.
        //  Throws ArgumentException if value is null.
        //
        ////////////////////////////////////////////////////////////////////////


        public virtual int LastIndexOf(string source, char value)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            // Can't start at negative index, so make sure we check for the length == 0 case.
            return LastIndexOf(source, value, source.Length - 1, source.Length, CompareOptions.None);
        }


        public virtual int LastIndexOf(string source, string value)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            // Can't start at negative index, so make sure we check for the length == 0 case.
            return LastIndexOf(source, value, source.Length - 1,
                source.Length, CompareOptions.None);
        }


        public virtual int LastIndexOf(string source, char value, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            // Can't start at negative index, so make sure we check for the length == 0 case.
            return LastIndexOf(source, value, source.Length - 1,
                source.Length, options);
        }

        public virtual int LastIndexOf(string source, string value, CompareOptions options)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            // Can't start at negative index, so make sure we check for the length == 0 case.
            return LastIndexOf(source, value, source.Length - 1, source.Length, options);
        }

        public virtual int LastIndexOf(string source, char value, int startIndex)
        {
            return LastIndexOf(source, value, startIndex, startIndex + 1, CompareOptions.None);
        }


        public virtual int LastIndexOf(string source, string value, int startIndex)
        {
            return LastIndexOf(source, value, startIndex, startIndex + 1, CompareOptions.None);
        }

        public virtual int LastIndexOf(string source, char value, int startIndex, CompareOptions options)
        {
            return LastIndexOf(source, value, startIndex, startIndex + 1, options);
        }


        public virtual int LastIndexOf(string source, string value, int startIndex, CompareOptions options)
        {
            return LastIndexOf(source, value, startIndex, startIndex + 1, options);
        }


        public virtual int LastIndexOf(string source, char value, int startIndex, int count)
        {
            return LastIndexOf(source, value, startIndex, count, CompareOptions.None);
        }


        public virtual int LastIndexOf(string source, string value, int startIndex, int count)
        {
            return LastIndexOf(source, value, startIndex, count, CompareOptions.None);
        }


        public virtual int LastIndexOf(string source, char value, int startIndex, int count, CompareOptions options)
        {
            // Verify Arguments
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            // Validate CompareOptions
            // Ordinal can't be selected with other flags
            if ((options & ValidIndexMaskOffFlags) != 0 &&
                (options != CompareOptions.Ordinal) &&
                (options != CompareOptions.OrdinalIgnoreCase))
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));

            // Special case for 0 length input strings
            if (source.Length == 0 && (startIndex == -1 || startIndex == 0))
                return -1;

            // Make sure we're not out of range
            if (startIndex < 0 || startIndex > source.Length)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);

            // Make sure that we allow startIndex == source.Length
            if (startIndex == source.Length)
            {
                startIndex--;
                if (count > 0)
                    count--;
            }

            // 2nd have of this also catches when startIndex == MAXINT, so MAXINT - 0 + 1 == -1, which is < 0.
            if (count < 0 || startIndex - count + 1 < 0)
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_Count);

            return InvariantLastIndexOf(source, new string(value, 1), startIndex, count, (options & (CompareOptions.IgnoreCase | CompareOptions.OrdinalIgnoreCase)) != 0);
        }


        public virtual int LastIndexOf(string source, string value, int startIndex, int count, CompareOptions options)
        {
            // Verify Arguments
            if (source == null)
                throw new ArgumentNullException(nameof(source));
            if (value == null)
                throw new ArgumentNullException(nameof(value));

            // Validate CompareOptions
            // Ordinal can't be selected with other flags
            if ((options & ValidIndexMaskOffFlags) != 0 &&
                (options != CompareOptions.Ordinal) &&
                (options != CompareOptions.OrdinalIgnoreCase))
                throw new ArgumentException(SR.Argument_InvalidFlag, nameof(options));

            // Special case for 0 length input strings
            if (source.Length == 0 && (startIndex == -1 || startIndex == 0))
                return (value.Length == 0) ? 0 : -1;

            // Make sure we're not out of range
            if (startIndex < 0 || startIndex > source.Length)
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_Index);

            // Make sure that we allow startIndex == source.Length
            if (startIndex == source.Length)
            {
                startIndex--;
                if (count > 0)
                    count--;

                // If we are looking for nothing, just return 0
                if (value.Length == 0 && count >= 0 && startIndex - count + 1 >= 0)
                    return startIndex;
            }

            // 2nd half of this also catches when startIndex == MAXINT, so MAXINT - 0 + 1 == -1, which is < 0.
            if (count < 0 || startIndex - count + 1 < 0)
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_Count);

            if (options == CompareOptions.OrdinalIgnoreCase)
            {
                return LastIndexOfOrdinal(source, value, startIndex, count, ignoreCase: true);
            }

            return InvariantLastIndexOf(source, value, startIndex, count, (options & (CompareOptions.IgnoreCase | CompareOptions.OrdinalIgnoreCase)) != 0);
        }

        internal int LastIndexOfOrdinal(string source, string value, int startIndex, int count, bool ignoreCase)
        {
            return InvariantLastIndexOf(source, value, startIndex, count, ignoreCase);
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  Equals
        //
        //  Implements Object.Equals().  Returns a boolean indicating whether
        //  or not object refers to the same CompareInfo as the current
        //  instance.
        //
        ////////////////////////////////////////////////////////////////////////


        public override bool Equals(object value)
        {
            CompareInfo that = value as CompareInfo;

            if (that != null)
            {
                return this.Name == that.Name;
            }

            return (false);
        }


        ////////////////////////////////////////////////////////////////////////
        //
        //  GetHashCode
        //
        //  Implements Object.GetHashCode().  Returns the hash code for the
        //  CompareInfo.  The hash code is guaranteed to be the same for
        //  CompareInfo A and B where A.Equals(B) is true.
        //
        ////////////////////////////////////////////////////////////////////////


        public override int GetHashCode()
        {
            return (this.Name.GetHashCode());
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  ToString
        //
        //  Implements Object.ToString().  Returns a string describing the
        //  CompareInfo.
        //
        ////////////////////////////////////////////////////////////////////////
        public override string ToString()
        {
            return ("CompareInfo - " + this.Name);
        }
    }
}
