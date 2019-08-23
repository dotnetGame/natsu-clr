using System;
using System.Diagnostics.CodeAnalysis;
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
        public static void WriteLine(string? message)
        {
            Write(message + Environment.NewLine);
        }

        private static bool s_needIndent;

        private static string s_indentString;

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Close() { }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Flush() { }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Indent()
        {
            IndentLevel++;
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Unindent()
        {
            IndentLevel--;
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Print(string? message)
        {
            WriteLine(message);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Print(string format, params object?[] args)
        {
            WriteLine(string.Format(null, format, args));
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Assert([DoesNotReturnIf(false)] bool condition)
        {
            Assert(condition, string.Empty, string.Empty);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Assert([DoesNotReturnIf(false)] bool condition, string? message)
        {
            Assert(condition, message, string.Empty);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Assert([DoesNotReturnIf(false)] bool condition, string? message, string? detailMessage)
        {
            if (!condition)
            {
                Fail(message, detailMessage);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        [DoesNotReturn]
        public static void Fail(string? message)
        {
            Fail(message, string.Empty);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        [DoesNotReturn]
        public static void Fail(string? message, string? detailMessage)
        {
            FailCore(message, detailMessage);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Assert([DoesNotReturnIf(false)] bool condition, string? message, string detailMessageFormat, params object?[] args)
        {
            Assert(condition, message, string.Format(detailMessageFormat, args));
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(string? message)
        {
            // TODO: implement lock
            //lock (s_lock)
            //{
                WriteCore(message);
            //}
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(object? value)
        {
            WriteLine(value?.ToString());
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(object? value, string? category)
        {
            WriteLine(value?.ToString(), category);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(string format, params object?[] args)
        {
            WriteLine(string.Format(null, format, args));
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLine(string? message, string? category)
        {
            if (category == null)
            {
                WriteLine(message);
            }
            else
            {
                WriteLine(category + ": " + message);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(object? value)
        {
            Write(value?.ToString());
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(string? message, string? category)
        {
            if (category == null)
            {
                Write(message);
            }
            else
            {
                Write(category + ": " + message);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void Write(object? value, string? category)
        {
            Write(value?.ToString(), category);
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, string? message)
        {
            if (condition)
            {
                Write(message);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, object? value)
        {
            if (condition)
            {
                Write(value);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, string? message, string? category)
        {
            if (condition)
            {
                Write(message, category);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteIf(bool condition, object? value, string? category)
        {
            if (condition)
            {
                Write(value, category);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, object? value)
        {
            if (condition)
            {
                WriteLine(value);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, object? value, string? category)
        {
            if (condition)
            {
                WriteLine(value, category);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, string? message)
        {
            if (condition)
            {
                WriteLine(message);
            }
        }

        [System.Diagnostics.Conditional("DEBUG")]
        public static void WriteLineIf(bool condition, string? message, string? category)
        {
            if (condition)
            {
                WriteLine(message, category);
            }
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void WriteCore(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void FailCore(string message, string detailMessage);
    }
}
