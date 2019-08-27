using System;
using System.Diagnostics;

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

        public static string Concat(object? arg0) => arg0?.ToString() ?? string.Empty;

        public static string Concat(object? arg0, object? arg1)
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

        public static string Concat(object? arg0, object? arg1, object? arg2)
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

        public static string Concat(string? str0, string? str1)
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

        public static string Concat(string? str0, string? str1, string? str2)
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

        public static string Concat(string? str0, string? str1, string? str2, string? str3)
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

        public static string Format(string format, object[] args)
        {
            return format;
        }

        public static string Format(string format, object arg1)
        {
            return format;
        }

        public static string Format(string format, object arg1, object arg2)
        {
            return format;
        }

        public static string Format(string format, object arg1, object arg2, object arg3)
        {
            return format;
        }

        internal static string Join(string v, object[] args)
        {
            return v;
        }

        internal static string Join(string v, string format, object arg1)
        {
            return format;
        }

        internal static string Join(string v, string format, object arg1, object arg2)
        {
            return format;
        }

        internal static string Join(string v, string format, object arg1, object arg2, object arg3)
        {
            return format;
        }

        internal static string Format(IFormatProvider provider, string format, params object[] args)
        {
            return format;
        }
    }
}
