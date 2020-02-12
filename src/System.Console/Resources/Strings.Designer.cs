namespace System
{
    internal static partial class SR
    {
        public static string ArgumentOutOfRange_ConsoleWindowBufferSize => @"The value must be less than the console's current maximum window size of {0} in that dimension. Note that this value depends on screen resolution and the console font.";
        public static string ArgumentOutOfRange_ConsoleWindowSize_Size => @"The new console window size would force the console buffer size to be too large.";
        public static string ArgumentOutOfRange_NeedNonNegNum => @"Non-negative number required.";
        public static string ArgumentOutOfRange_NeedPosNum => @"Positive number required.";
        public static string ArgumentNull_Buffer => @"Buffer cannot be null.";
        public static string Argument_InvalidOffLen => @"Offset and length were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.";
        public static string ArgumentOutOfRange_FileLengthTooBig => @"Specified file length was too large for the file system.";
        public static string NotSupported_UnseekableStream => @"Stream does not support seeking.";
        public static string ObjectDisposed_FileClosed => @"Cannot access a closed file.";
        public static string NotSupported_UnwritableStream => @"Stream does not support writing.";
        public static string NotSupported_UnreadableStream => @"Stream does not support reading.";
        public static string IO_AlreadyExists_Name => @"Cannot create '{0}' because a file or directory with the same name already exists.";
        public static string IO_FileExists_Name => @"The file '{0}' already exists.";
        public static string IO_FileNotFound => @"Unable to find the specified file.";
        public static string IO_FileNotFound_FileName => @"Could not find file '{0}'.";
        public static string IO_PathNotFound_NoPathName => @"Could not find a part of the path.";
        public static string IO_PathNotFound_Path => @"Could not find a part of the path '{0}'.";
        public static string IO_PathTooLong => @"The specified file name or path is too long, or a component of the specified path is too long.";
        public static string UnauthorizedAccess_IODenied_NoPathName => @"Access to the path is denied.";
        public static string UnauthorizedAccess_IODenied_Path => @"Access to the path '{0}' is denied.";
        public static string IO_SharingViolation_File => @"The process cannot access the file '{0}' because it is being used by another process.";
        public static string IO_SharingViolation_NoFileName => @"The process cannot access the file because it is being used by another process.";
        public static string IndexOutOfRange_IORaceCondition => @"Probable I/O race condition detected while copying memory. The I/O package is not thread safe by default. In multithreaded applications, a stream must be accessed in a thread-safe way, such as a thread-safe wrapper returned by TextReader's or TextWriter's Synchronized methods. This also applies to classes like StreamWriter and StreamReader.";
        public static string Arg_InvalidConsoleColor => @"The ConsoleColor enum value was not defined on that enum. Please use a defined color from the enum.";
        public static string IO_NoConsole => @"There is no console.";
        public static string IO_TermInfoInvalid => @"The terminfo database is invalid.";
        public static string InvalidOperation_PrintF => @"The printf operation failed.";
        public static string InvalidOperation_ConsoleReadKeyOnFile => @"Cannot read keys when either application does not have a console or when console input has been redirected. Try Console.Read.";
        public static string PersistedFiles_NoHomeDirectory => @"The home directory of the current user could not be determined.";
        public static string ArgumentOutOfRange_ConsoleKey => @"Console key values must be between 0 and 255 inclusive.";
        public static string ArgumentOutOfRange_ConsoleBufferBoundaries => @"The value must be greater than or equal to zero and less than the console's buffer size in that dimension.";
        public static string ArgumentOutOfRange_ConsoleWindowPos => @"The window position must be set such that the current window size fits within the console's buffer, and the numbers must not be negative.";
        public static string InvalidOperation_ConsoleKeyAvailableOnFile => @"Cannot see if a key has been pressed when either application does not have a console or when console input has been redirected from a file. Try Console.In.Peek.";
        public static string ArgumentOutOfRange_ConsoleBufferLessThanWindowSize => @"The console buffer size must not be less than the current size and position of the console window, nor greater than or equal to short.MaxValue.";
        public static string ArgumentOutOfRange_CursorSize => @"The cursor size is invalid. It must be a percentage between 1 and 100.";
        public static string ArgumentOutOfRange_BeepFrequency => @"The frequency must be between {0} and {1}.";
        public static string ArgumentNull_Array => @"Array cannot be null.";
        public static string ArgumentOutOfRange_IndexCountBuffer => @"Index and count must refer to a location within the buffer.";
        public static string ArgumentOutOfRange_IndexCount => @"Index and count must refer to a location within the string.";
        public static string ArgumentOutOfRange_Index => @"Index was out of range. Must be non-negative and less than the size of the collection.";
        public static string Argument_EncodingConversionOverflowBytes => @"The output byte buffer is too small to contain the encoded data, encoding '{0}' fallback '{1}'.";
        public static string Argument_EncodingConversionOverflowChars => @"The output char buffer is too small to contain the decoded characters, encoding '{0}' fallback '{1}'.";
        public static string ArgumentOutOfRange_GetByteCountOverflow => @"Too many characters. The resulting number of bytes is larger than what can be returned as an int.";
        public static string ArgumentOutOfRange_GetCharCountOverflow => @"Too many bytes. The resulting number of chars is larger than what can be returned as an int.";
        public static string Argument_InvalidCharSequenceNoIndex => @"String contains invalid Unicode code points.";
        public static string IO_PathTooLong_Path => @"The path '{0}' is too long, or a component of the specified path is too long.";
        public static string IO_TermInfoInvalidMagicNumber => @"The terminfo database has an invalid magic number: '{0}'.";
    }
}
