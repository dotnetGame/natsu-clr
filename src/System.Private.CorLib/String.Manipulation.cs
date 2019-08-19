using System;
using System.Diagnostics;

namespace System
{
    public partial class String
    {
        private const int StackallocIntBufferSizeLimit = 128;

        private static unsafe void FillStringChecked(string dest, int destPos, string src)
        {
            // TODO
            // Debug.Assert(dest != null);
            // Debug.Assert(src != null);
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
    }
}
