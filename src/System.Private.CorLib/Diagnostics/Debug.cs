using System;
using System.Runtime.CompilerServices;

namespace System.Diagnostics
{
    public static class Debug
    {
        //[ThreadStatic]
        private static int s_indentLevel;
        public static int IndentLevel
        {
            get
            {
                return s_indentLevel;
            }
            set
            {
                s_indentLevel = value < 0 ? 0 : value;
            }
        }

        private static int s_indentSize = 4;
        public static int IndentSize
        {
            get
            {
                return s_indentSize;
            }
            set
            {
                s_indentSize = value < 0 ? 0 : value;
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(string message)
        {
            Write(message + Environment.NewLine);
        }

        private static bool s_needIndent;

        private static string s_indentString;

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(string message)
        {
            // TODO: implement lock
            //lock (s_lock)
            //{
                WriteCore(message);
            //}
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(object value)
        {
            WriteLine(value?.ToString());
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(object value)
        {
            Write(value?.ToString());
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(object value, string category)
        {
            Write(value?.ToString(), category);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, string message)
        {
            if (condition)
            {
                Write(message);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, object value)
        {
            if (condition)
            {
                Write(value);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, string message, string category)
        {
            if (condition)
            {
                Write(message, category);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, object value, string category)
        {
            if (condition)
            {
                Write(value, category);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, object value)
        {
            if (condition)
            {
                WriteLine(value);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, string message)
        {
            if (condition)
            {
                WriteLine(message);
            }
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void WriteCore(string message);
    }
}
