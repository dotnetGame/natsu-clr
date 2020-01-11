using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace System
{
    public partial class String
    {
        private const int StackallocIntBufferSizeLimit = 128;

        private static unsafe void FillStringChecked(string dest, int destPos, string src)
        {
            Debug.Assert(dest != null);
            Debug.Assert(src != null);
            if (src.Length > dest.Length - destPos)
            {
                throw new IndexOutOfRangeException();
            }

            fixed (char* pDest = &dest._firstChar)
            fixed (char* pSrc = &src._firstChar)
            {
                wstrcpy(pDest + destPos, pSrc, src.Length);
            }
        }

        public static string Concat(object arg0)
        {
            if (arg0 == null)
            {
                return string.Empty;
            }
            return arg0.ToString();
        }

        public static string Concat(object arg0, object arg1)
        {
            if (arg0 == null)
            {
                arg0 = string.Empty;
            }

            if (arg1 == null)
            {
                arg1 = string.Empty;
            }
            return Concat(arg0.ToString(), arg1.ToString());
        }

        public static string Concat(object arg0, object arg1, object arg2)
        {
            if (arg0 == null)
            {
                arg0 = string.Empty;
            }

            if (arg1 == null)
            {
                arg1 = string.Empty;
            }

            if (arg2 == null)
            {
                arg2 = string.Empty;
            }

            return Concat(arg0.ToString(), arg1.ToString(), arg2.ToString());
        }

        public static string Concat(params object[] args)
        {
            if (args == null)
            {
                throw new ArgumentNullException(nameof(args));
            }

            if (args.Length <= 1)
            {
                return args.Length == 0 ?
                    string.Empty :
                    args[0]?.ToString() ?? string.Empty;
            }

            // We need to get an intermediary string array
            // to fill with each of the args' ToString(),
            // and then just concat that in one operation.

            // This way we avoid any intermediary string representations,
            // or buffer resizing if we use StringBuilder (although the
            // latter case is partially alleviated due to StringBuilder's
            // linked-list style implementation)

            var strings = new string[args.Length];

            int totalLength = 0;

            for (int i = 0; i < args.Length; i++)
            {
                object value = args[i];

                string toString = value?.ToString() ?? string.Empty; // We need to handle both the cases when value or value.ToString() is null
                strings[i] = toString;

                totalLength += toString.Length;

                if (totalLength < 0) // Check for a positive overflow
                {
                    throw new OutOfMemoryException();
                }
            }

            // If all of the ToStrings are null/empty, just return string.Empty
            if (totalLength == 0)
            {
                return string.Empty;
            }

            string result = FastAllocateString(totalLength);
            int position = 0; // How many characters we've copied so far

            for (int i = 0; i < strings.Length; i++)
            {
                string s = strings[i];

                Debug.Assert(s != null);
                Debug.Assert(position <= totalLength - s.Length, "We didn't allocate enough space for the result string!");

                FillStringChecked(result, position, s);
                position += s.Length;
            }

            return result;
        }

        public static string Concat(IEnumerable<string> values)
        {
            if (values == null)
                throw new ArgumentNullException(nameof(values));

            using (IEnumerator<string> en = values.GetEnumerator())
            {
                if (!en.MoveNext())
                    return string.Empty;

                string firstValue = en.Current;

                if (!en.MoveNext())
                {
                    return firstValue ?? string.Empty;
                }

                StringBuilder result = StringBuilderCache.Acquire();
                result.Append(firstValue);

                do
                {
                    result.Append(en.Current);
                }
                while (en.MoveNext());

                return StringBuilderCache.GetStringAndRelease(result);
            }
        }

        public static string Concat(string str0, string str1)
        {
            if (IsNullOrEmpty(str0))
            {
                if (IsNullOrEmpty(str1))
                {
                    return string.Empty;
                }
                return str1;
            }

            if (IsNullOrEmpty(str1))
            {
                return str0;
            }

            int str0Length = str0.Length;

            string result = FastAllocateString(str0Length + str1.Length);

            FillStringChecked(result, 0, str0);
            FillStringChecked(result, str0Length, str1);

            return result;
        }

        public static string Concat(string str0, string str1, string str2)
        {
            if (IsNullOrEmpty(str0))
            {
                return Concat(str1, str2);
            }

            if (IsNullOrEmpty(str1))
            {
                return Concat(str0, str2);
            }

            if (IsNullOrEmpty(str2))
            {
                return Concat(str0, str1);
            }

            int totalLength = str0.Length + str1.Length + str2.Length;

            string result = FastAllocateString(totalLength);
            FillStringChecked(result, 0, str0);
            FillStringChecked(result, str0.Length, str1);
            FillStringChecked(result, str0.Length + str1.Length, str2);

            return result;
        }

        public static string Concat(string str0, string str1, string str2, string str3)
        {
            if (IsNullOrEmpty(str0))
            {
                return Concat(str1, str2, str3);
            }

            if (IsNullOrEmpty(str1))
            {
                return Concat(str0, str2, str3);
            }

            if (IsNullOrEmpty(str2))
            {
                return Concat(str0, str1, str3);
            }

            if (IsNullOrEmpty(str3))
            {
                return Concat(str0, str1, str2);
            }

            int totalLength = str0.Length + str1.Length + str2.Length + str3.Length;

            string result = FastAllocateString(totalLength);
            FillStringChecked(result, 0, str0);
            FillStringChecked(result, str0.Length, str1);
            FillStringChecked(result, str0.Length + str1.Length, str2);
            FillStringChecked(result, str0.Length + str1.Length + str2.Length, str3);

            return result;
        }

        public static string Concat(params string[] values)
        {
            if (values == null)
                throw new ArgumentNullException(nameof(values));

            if (values.Length <= 1)
            {
                return values.Length == 0 ?
                    string.Empty :
                    values[0] ?? string.Empty;
            }

            // It's possible that the input values array could be changed concurrently on another
            // thread, such that we can't trust that each read of values[i] will be equivalent.
            // Worst case, we can make a defensive copy of the array and use that, but we first
            // optimistically try the allocation and copies assuming that the array isn't changing,
            // which represents the 99.999% case, in particular since string.Concat is used for
            // string concatenation by the languages, with the input array being a params array.

            // Sum the lengths of all input strings
            long totalLengthLong = 0;
            for (int i = 0; i < values.Length; i++)
            {
                string value = values[i];
                if (value != null)
                {
                    totalLengthLong += value.Length;
                }
            }

            // If it's too long, fail, or if it's empty, return an empty string.
            if (totalLengthLong > int.MaxValue)
            {
                throw new OutOfMemoryException();
            }
            int totalLength = (int)totalLengthLong;
            if (totalLength == 0)
            {
                return string.Empty;
            }

            // Allocate a new string and copy each input string into it
            string result = FastAllocateString(totalLength);
            int copiedLength = 0;
            for (int i = 0; i < values.Length; i++)
            {
                string value = values[i];
                if (!string.IsNullOrEmpty(value))
                {
                    int valueLen = value.Length;
                    if (valueLen > totalLength - copiedLength)
                    {
                        copiedLength = -1;
                        break;
                    }

                    FillStringChecked(result, copiedLength, value);
                    copiedLength += valueLen;
                }
            }

            // If we copied exactly the right amount, return the new string.  Otherwise,
            // something changed concurrently to mutate the input array: fall back to
            // doing the concatenation again, but this time with a defensive copy. This
            // fall back should be extremely rare.
            return copiedLength == totalLength ? result : Concat((string[])values.Clone());
        }

        public static string Format(string format, object arg0)
        {
            return FormatHelper(null, format, new ParamsArray(arg0));
        }

        public static string Format(string format, object arg0, object arg1)
        {
            return FormatHelper(null, format, new ParamsArray(arg0, arg1));
        }

        public static string Format(string format, object arg0, object arg1, object arg2)
        {
            return FormatHelper(null, format, new ParamsArray(arg0, arg1, arg2));
        }

        public static string Format(string format, params object[] args)
        {
            if (args == null)
            {
                // To preserve the original exception behavior, throw an exception about format if both
                // args and format are null. The actual null check for format is in FormatHelper.
                throw new ArgumentNullException((format == null) ? nameof(format) : nameof(args));
            }

            return FormatHelper(null, format, new ParamsArray(args));
        }

        public static string Format(IFormatProvider provider, string format, object arg0)
        {
            return FormatHelper(provider, format, new ParamsArray(arg0));
        }

        public static string Format(IFormatProvider provider, string format, object arg0, object arg1)
        {
            return FormatHelper(provider, format, new ParamsArray(arg0, arg1));
        }

        public static string Format(IFormatProvider provider, string format, object arg0, object arg1, object arg2)
        {
            return FormatHelper(provider, format, new ParamsArray(arg0, arg1, arg2));
        }

        public static string Format(IFormatProvider provider, string format, params object[] args)
        {
            if (args == null)
            {
                // To preserve the original exception behavior, throw an exception about format if both
                // args and format are null. The actual null check for format is in FormatHelper.
                throw new ArgumentNullException((format == null) ? nameof(format) : nameof(args));
            }

            return FormatHelper(provider, format, new ParamsArray(args));
        }

        private static string FormatHelper(IFormatProvider provider, string format, ParamsArray args)
        {
            if (format == null)
                throw new ArgumentNullException(nameof(format));

            return StringBuilderCache.GetStringAndRelease(
                StringBuilderCache
                    .Acquire(format.Length + args.Length * 8)
                    .AppendFormatHelper(provider, format, args));
        }

        public string Insert(int startIndex, string value)
        {
            if (value == null)
                throw new ArgumentNullException(nameof(value));
            if (startIndex < 0 || startIndex > this.Length)
                throw new ArgumentOutOfRangeException(nameof(startIndex));

            int oldLength = Length;
            int insertLength = value.Length;

            if (oldLength == 0)
                return value;
            if (insertLength == 0)
                return this;

            // In case this computation overflows, newLength will be negative and FastAllocateString throws OutOfMemoryException
            int newLength = oldLength + insertLength;
            string result = FastAllocateString(newLength);
            unsafe
            {
                fixed (char* srcThis = &_firstChar)
                {
                    fixed (char* srcInsert = &value._firstChar)
                    {
                        fixed (char* dst = &result._firstChar)
                        {
                            wstrcpy(dst, srcThis, startIndex);
                            wstrcpy(dst + startIndex, srcInsert, insertLength);
                            wstrcpy(dst + startIndex + insertLength, srcThis + startIndex, oldLength - startIndex);
                        }
                    }
                }
            }
            return result;
        }

        public static string Join(char separator, params string[] value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }

            return Join(separator, value, 0, value.Length);
        }

        public static unsafe string Join(char separator, params object[] values)
        {
            // Defer argument validation to the internal function
            return JoinCore(&separator, 1, values);
        }

        public static unsafe string Join<T>(char separator, IEnumerable<T> values)
        {
            // Defer argument validation to the internal function
            return JoinCore(&separator, 1, values);
        }

        public static unsafe string Join(char separator, string[] value, int startIndex, int count)
        {
            // Defer argument validation to the internal function
            return JoinCore(&separator, 1, value, startIndex, count);
        }

        // Joins an array of strings together as one string with a separator between each original string.
        //
        public static string Join(string separator, params string[] value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }
            return Join(separator, value, 0, value.Length);
        }

        public static unsafe string Join(string separator, params object[] values)
        {
            separator = separator ?? string.Empty;
            fixed (char* pSeparator = &separator._firstChar)
            {
                // Defer argument validation to the internal function
                return JoinCore(pSeparator, separator.Length, values);
            }
        }

        public static unsafe string Join<T>(string separator, IEnumerable<T> values)
        {
            separator = separator ?? string.Empty;
            fixed (char* pSeparator = &separator._firstChar)
            {
                // Defer argument validation to the internal function
                return JoinCore(pSeparator, separator.Length, values);
            }
        }

        public static string Join(string separator, IEnumerable<string> values)
        {
            if (values == null)
            {
                throw new ArgumentNullException(nameof(values));
            }

            using (IEnumerator<string> en = values.GetEnumerator())
            {
                if (!en.MoveNext())
                {
                    return string.Empty;
                }

                string firstValue = en.Current;

                if (!en.MoveNext())
                {
                    // Only one value available
                    return firstValue ?? string.Empty;
                }

                // Null separator and values are handled by the StringBuilder
                StringBuilder result = StringBuilderCache.Acquire();
                result.Append(firstValue);

                do
                {
                    result.Append(separator);
                    result.Append(en.Current);
                }
                while (en.MoveNext());

                return StringBuilderCache.GetStringAndRelease(result);
            }
        }

        // Joins an array of strings together as one string with a separator between each original string.
        //
        public static unsafe string Join(string separator, string[] value, int startIndex, int count)
        {
            separator = separator ?? string.Empty;
            fixed (char* pSeparator = &separator._firstChar)
            {
                // Defer argument validation to the internal function
                return JoinCore(pSeparator, separator.Length, value, startIndex, count);
            }
        }

        private static unsafe string JoinCore(char* separator, int separatorLength, object[] values)
        {
            if (values == null)
            {
                throw new ArgumentNullException(nameof(values));
            }

            if (values.Length == 0)
            {
                return string.Empty;
            }

            string firstString = values[0]?.ToString();

            if (values.Length == 1)
            {
                return firstString ?? string.Empty;
            }

            StringBuilder result = StringBuilderCache.Acquire();
            result.Append(firstString);

            for (int i = 1; i < values.Length; i++)
            {
                result.Append(separator, separatorLength);
                object value = values[i];
                if (value != null)
                {
                    result.Append(value.ToString());
                }
            }

            return StringBuilderCache.GetStringAndRelease(result);
        }

        private static unsafe string JoinCore<T>(char* separator, int separatorLength, IEnumerable<T> values)
        {
            if (values == null)
            {
                throw new ArgumentNullException(nameof(values));
            }

            using (IEnumerator<T> en = values.GetEnumerator())
            {
                if (!en.MoveNext())
                {
                    return string.Empty;
                }

                // We called MoveNext once, so this will be the first item
                T currentValue = en.Current;

                // Call ToString before calling MoveNext again, since
                // we want to stay consistent with the below loop
                // Everything should be called in the order
                // MoveNext-Current-ToString, unless further optimizations
                // can be made, to avoid breaking changes
                string firstString = currentValue?.ToString();

                // If there's only 1 item, simply call ToString on that
                if (!en.MoveNext())
                {
                    // We have to handle the case of either currentValue
                    // or its ToString being null
                    return firstString ?? string.Empty;
                }

                StringBuilder result = StringBuilderCache.Acquire();

                result.Append(firstString);

                do
                {
                    currentValue = en.Current;

                    result.Append(separator, separatorLength);
                    if (currentValue != null)
                    {
                        result.Append(currentValue.ToString());
                    }
                }
                while (en.MoveNext());

                return StringBuilderCache.GetStringAndRelease(result);
            }
        }

        private static unsafe string JoinCore(char* separator, int separatorLength, string[] value, int startIndex, int count)
        {
            // If the separator is null, it is converted to an empty string before entering this function.
            // Even for empty strings, fixed should never return null (it should return a pointer to a null char).
            Debug.Assert(separator != null);
            Debug.Assert(separatorLength >= 0);

            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }
            if (startIndex < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_StartIndex);
            }
            if (count < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_NegativeCount);
            }
            if (startIndex > value.Length - count)
            {
                throw new ArgumentOutOfRangeException(nameof(startIndex), SR.ArgumentOutOfRange_IndexCountBuffer);
            }

            if (count <= 1)
            {
                return count == 0 ?
                    string.Empty :
                    value[startIndex] ?? string.Empty;
            }

            long totalSeparatorsLength = (long)(count - 1) * separatorLength;
            if (totalSeparatorsLength > int.MaxValue)
            {
                throw new OutOfMemoryException();
            }
            int totalLength = (int)totalSeparatorsLength;

            // Calculate the length of the resultant string so we know how much space to allocate.
            for (int i = startIndex, end = startIndex + count; i < end; i++)
            {
                string currentValue = value[i];
                if (currentValue != null)
                {
                    totalLength += currentValue.Length;
                    if (totalLength < 0) // Check for overflow
                    {
                        throw new OutOfMemoryException();
                    }
                }
            }

            // Copy each of the strings into the resultant buffer, interleaving with the separator.
            string result = FastAllocateString(totalLength);
            int copiedLength = 0;

            for (int i = startIndex, end = startIndex + count; i < end; i++)
            {
                // It's possible that another thread may have mutated the input array
                // such that our second read of an index will not be the same string
                // we got during the first read.

                // We range check again to avoid buffer overflows if this happens.

                string currentValue = value[i];
                if (currentValue != null)
                {
                    int valueLen = currentValue.Length;
                    if (valueLen > totalLength - copiedLength)
                    {
                        copiedLength = -1;
                        break;
                    }

                    // Fill in the value.
                    FillStringChecked(result, copiedLength, currentValue);
                    copiedLength += valueLen;
                }

                if (i < end - 1)
                {
                    // Fill in the separator.
                    fixed (char* pResult = &result._firstChar)
                    {
                        // If we are called from the char-based overload, we will not
                        // want to call MemoryCopy each time we fill in the separator. So
                        // specialize for 1-length separators.
                        if (separatorLength == 1)
                        {
                            pResult[copiedLength] = *separator;
                        }
                        else
                        {
                            wstrcpy(pResult + copiedLength, separator, separatorLength);
                        }
                    }
                    copiedLength += separatorLength;
                }
            }

            // If we copied exactly the right amount, return the new string.  Otherwise,
            // something changed concurrently to mutate the input array: fall back to
            // doing the concatenation again, but this time with a defensive copy. This
            // fall back should be extremely rare.
            return copiedLength == totalLength ?
                result :
                JoinCore(separator, separatorLength, (string[])value.Clone(), startIndex, count);
        }
    }
}
