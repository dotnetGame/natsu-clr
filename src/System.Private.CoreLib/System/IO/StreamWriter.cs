// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace System.IO
{
    // This class implements a TextWriter for writing characters to a Stream.
    // This is designed for character output in a particular Encoding, 
    // whereas the Stream class is designed for byte input and output.  
    public class StreamWriter : TextWriter
    {
        // For UTF-8, the values of 1K for the default buffer size and 4K for the
        // file stream buffer size are reasonable & give very reasonable
        // performance for in terms of construction time for the StreamWriter and
        // write perf.  Note that for UTF-8, we end up allocating a 4K byte buffer,
        // which means we take advantage of adaptive buffering code.
        // The performance using UnicodeEncoding is acceptable.  
        private const int DefaultBufferSize = 1024;   // char[]
        private const int DefaultFileStreamBufferSize = 4096;
        private const int MinBufferSize = 128;

        private const int DontCopyOnWriteLineThreshold = 512;

        // Bit bucket - Null has no backing store. Non closable.
        public new static readonly StreamWriter Null = new StreamWriter(Stream.Null, UTF8NoBOM, MinBufferSize, true);

        private Stream _stream;
        private Encoding _encoding;
        private Encoder _encoder;
        private byte[] _byteBuffer;
        private char[] _charBuffer;
        private int _charPos;
        private int _charLen;
        private bool _autoFlush;
        private bool _haveWrittenPreamble;
        private bool _closable;

        private void CheckAsyncTaskInProgress()
        {
            // We are not locking the access to _asyncWriteTask because this is not meant to guarantee thread safety. 
            // We are simply trying to deter calling any Write APIs while an async Write from the same thread is in progress.
            if (false/*!_asyncWriteTask.IsCompleted*/)
            {
                ThrowAsyncIOInProgress();
            }
        }

        private static void ThrowAsyncIOInProgress() =>
            throw new InvalidOperationException(SR.InvalidOperation_AsyncIOInProgress);

        // The high level goal is to be tolerant of encoding errors when we read and very strict 
        // when we write. Hence, default StreamWriter encoding will throw on encoding error.   
        // Note: when StreamWriter throws on invalid encoding chars (for ex, high surrogate character 
        // D800-DBFF without a following low surrogate character DC00-DFFF), it will cause the 
        // internal StreamWriter's state to be irrecoverable as it would have buffered the 
        // illegal chars and any subsequent call to Flush() would hit the encoding error again. 
        // Even Close() will hit the exception as it would try to flush the unwritten data. 
        // Maybe we can add a DiscardBufferedData() method to get out of such situation (like 
        // StreamReader though for different reason). Either way, the buffered data will be lost!
        private static Encoding UTF8NoBOM => EncodingCache.UTF8NoBOM;


        internal StreamWriter() : base(null)
        { // Ask for CurrentCulture all the time 
        }

        public StreamWriter(Stream stream)
            : this(stream, UTF8NoBOM, DefaultBufferSize, false)
        {
        }

        public StreamWriter(Stream stream, Encoding encoding)
            : this(stream, encoding, DefaultBufferSize, false)
        {
        }

        // Creates a new StreamWriter for the given stream.  The 
        // character encoding is set by encoding and the buffer size, 
        // in number of 16-bit characters, is set by bufferSize.  
        // 
        public StreamWriter(Stream stream, Encoding encoding, int bufferSize)
            : this(stream, encoding, bufferSize, false)
        {
        }

        public StreamWriter(Stream stream, Encoding encoding, int bufferSize, bool leaveOpen)
            : base(null) // Ask for CurrentCulture all the time
        {
            if (stream == null || encoding == null)
            {
                throw new ArgumentNullException(stream == null ? nameof(stream) : nameof(encoding));
            }
            if (!stream.CanWrite)
            {
                throw new ArgumentException(SR.Argument_StreamNotWritable);
            }
            if (bufferSize <= 0)
            {
                throw new ArgumentOutOfRangeException(nameof(bufferSize), SR.ArgumentOutOfRange_NeedPosNum);
            }

            Init(stream, encoding, bufferSize, leaveOpen);
        }

        private void Init(Stream streamArg, Encoding encodingArg, int bufferSize, bool shouldLeaveOpen)
        {
            _stream = streamArg;
            _encoding = encodingArg;
            _encoder = _encoding.GetEncoder();
            if (bufferSize < MinBufferSize)
            {
                bufferSize = MinBufferSize;
            }

            _charBuffer = new char[bufferSize];
            _byteBuffer = new byte[_encoding.GetMaxByteCount(bufferSize)];
            _charLen = bufferSize;
            // If we're appending to a Stream that already has data, don't write
            // the preamble.
            if (_stream.CanSeek && _stream.Position > 0)
            {
                _haveWrittenPreamble = true;
            }

            _closable = !shouldLeaveOpen;
        }

        public override void Close()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected override void Dispose(bool disposing)
        {
            try
            {
                // We need to flush any buffered data if we are being closed/disposed.
                // Also, we never close the handles for stdout & friends.  So we can safely 
                // write any buffered data to those streams even during finalization, which 
                // is generally the right thing to do.
                if (_stream != null)
                {
                    // Note: flush on the underlying stream can throw (ex., low disk space)
                    if (disposing /* || (LeaveOpen && stream is __ConsoleStream) */)
                    {
                        CheckAsyncTaskInProgress();

                        Flush(true, true);
                    }
                }
            }
            finally
            {
                // Dispose of our resources if this StreamWriter is closable. 
                // Note: Console.Out and other such non closable streamwriters should be left alone 
                if (!LeaveOpen && _stream != null)
                {
                    try
                    {
                        // Attempt to close the stream even if there was an IO error from Flushing.
                        // Note that Stream.Close() can potentially throw here (may or may not be
                        // due to the same Flush error). In this case, we still need to ensure 
                        // cleaning up internal resources, hence the finally block.  
                        if (disposing)
                        {
                            _stream.Close();
                        }
                    }
                    finally
                    {
                        _stream = null;
                        _byteBuffer = null;
                        _charBuffer = null;
                        _encoding = null;
                        _encoder = null;
                        _charLen = 0;
                        base.Dispose(disposing);
                    }
                }
            }
        }

        public override void Flush()
        {
            CheckAsyncTaskInProgress();

            Flush(true, true);
        }

        private void Flush(bool flushStream, bool flushEncoder)
        {
            // flushEncoder should be true at the end of the file and if
            // the user explicitly calls Flush (though not if AutoFlush is true).
            // This is required to flush any dangling characters from our UTF-7 
            // and UTF-8 encoders.  
            if (_stream == null)
            {
                throw new ObjectDisposedException(null, SR.ObjectDisposed_WriterClosed);
            }

            // Perf boost for Flush on non-dirty writers.
            if (_charPos == 0 && !flushStream && !flushEncoder)
            {
                return;
            }

            if (!_haveWrittenPreamble)
            {
                _haveWrittenPreamble = true;
                ReadOnlySpan<byte> preamble = _encoding.Preamble;
                if (preamble.Length > 0)
                {
                    _stream.Write(preamble);
                }
            }

            int count = _encoder.GetBytes(_charBuffer, 0, _charPos, _byteBuffer, 0, flushEncoder);
            _charPos = 0;
            if (count > 0)
            {
                _stream.Write(_byteBuffer, 0, count);
            }
            // By definition, calling Flush should flush the stream, but this is
            // only necessary if we passed in true for flushStream.  The Web
            // Services guys have some perf tests where flushing needlessly hurts.
            if (flushStream)
            {
                _stream.Flush();
            }
        }

        public virtual bool AutoFlush
        {
            get { return _autoFlush; }

            set
            {
                CheckAsyncTaskInProgress();

                _autoFlush = value;
                if (value)
                {
                    Flush(true, false);
                }
            }
        }

        public virtual Stream BaseStream
        {
            get { return _stream; }
        }

        internal bool LeaveOpen
        {
            get { return !_closable; }
        }

        internal bool HaveWrittenPreamble
        {
            set { _haveWrittenPreamble = value; }
        }

        public override Encoding Encoding
        {
            get { return _encoding; }
        }

        public override void Write(char value)
        {
            CheckAsyncTaskInProgress();

            if (_charPos == _charLen)
            {
                Flush(false, false);
            }

            _charBuffer[_charPos] = value;
            _charPos++;
            if (_autoFlush)
            {
                Flush(true, false);
            }
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void Write(char[] buffer)
        {
            WriteSpan(buffer, appendNewLine: false);
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void Write(char[] buffer, int index, int count)
        {
            if (buffer == null)
            {
                throw new ArgumentNullException(nameof(buffer), SR.ArgumentNull_Buffer);
            }
            if (index < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(index), SR.ArgumentOutOfRange_NeedNonNegNum);
            }
            if (count < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(count), SR.ArgumentOutOfRange_NeedNonNegNum);
            }
            if (buffer.Length - index < count)
            {
                throw new ArgumentException(SR.Argument_InvalidOffLen);
            }

            WriteSpan(buffer.AsSpan(index, count), appendNewLine: false);
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void Write(ReadOnlySpan<char> buffer)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteSpan(buffer, appendNewLine: false);
            }
            else
            {
                // If a derived class may have overridden existing Write behavior,
                // we need to make sure we use it.
                base.Write(buffer);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private unsafe void WriteSpan(ReadOnlySpan<char> buffer, bool appendNewLine)
        {
            CheckAsyncTaskInProgress();

            if (buffer.Length <= 4 && // Threshold of 4 chosen based on perf experimentation
                buffer.Length <= _charLen - _charPos)
            {
                // For very short buffers and when we don't need to worry about running out of space
                // in the char buffer, just copy the chars individually.
                for (int i = 0; i < buffer.Length; i++)
                {
                    _charBuffer[_charPos++] = buffer[i];
                }
            }
            else
            {
                // For larger buffers or when we may run out of room in the internal char buffer, copy in chunks.
                // Use unsafe code until https://github.com/dotnet/coreclr/issues/13827 is addressed, as spans are
                // resulting in significant overhead (even when the if branch above is taken rather than this
                // else) due to temporaries that need to be cleared.  Given the use of unsafe code, we also
                // make local copies of instance state to protect against potential concurrent misuse.

                char[] charBuffer = _charBuffer;
                if (charBuffer == null)
                {
                    throw new ObjectDisposedException(null, SR.ObjectDisposed_WriterClosed);
                }

                fixed (char* bufferPtr = &MemoryMarshal.GetReference(buffer))
                fixed (char* dstPtr = &charBuffer[0])
                {
                    char* srcPtr = bufferPtr;
                    int count = buffer.Length;
                    int dstPos = _charPos; // use a local copy of _charPos for safety
                    while (count > 0)
                    {
                        if (dstPos == charBuffer.Length)
                        {
                            Flush(false, false);
                            dstPos = 0;
                        }

                        int n = Math.Min(charBuffer.Length - dstPos, count);
                        int bytesToCopy = n * sizeof(char);

                        Buffer.MemoryCopy(srcPtr, dstPtr + dstPos, bytesToCopy, bytesToCopy);

                        _charPos += n;
                        dstPos += n;
                        srcPtr += n;
                        count -= n;
                    }
                }
            }

            if (appendNewLine)
            {
                char[] coreNewLine = CoreNewLine;
                for (int i = 0; i < coreNewLine.Length; i++) // Generally 1 (\n) or 2 (\r\n) iterations
                {
                    if (_charPos == _charLen)
                    {
                        Flush(false, false);
                    }

                    _charBuffer[_charPos] = coreNewLine[i];
                    _charPos++;
                }
            }

            if (_autoFlush)
            {
                Flush(true, false);
            }
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void Write(string value)
        {
            WriteSpan(value, appendNewLine: false);
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void WriteLine(string value)
        {
            CheckAsyncTaskInProgress();
            WriteSpan(value, appendNewLine: true);
        }

        [MethodImpl(MethodImplOptions.NoInlining)] // prevent WriteSpan from bloating call sites
        public override void WriteLine(ReadOnlySpan<char> value)
        {
            if (GetType() == typeof(StreamWriter))
            {
                CheckAsyncTaskInProgress();
                WriteSpan(value, appendNewLine: true);
            }
            else
            {
                // If a derived class may have overridden existing WriteLine behavior,
                // we need to make sure we use it.
                base.WriteLine(value);
            }
        }

        private void WriteFormatHelper(string format, ParamsArray args, bool appendNewLine)
        {
            StringBuilder sb =
                StringBuilderCache.Acquire(format.Length + args.Length * 8)
                .AppendFormatHelper(null, format, args);

            StringBuilder.ChunkEnumerator chunks = sb.GetChunks();

            bool more = chunks.MoveNext();
            while (more)
            {
                ReadOnlySpan<char> current = chunks.Current.Span;
                more = chunks.MoveNext();

                // If final chunk, include the newline if needed
                WriteSpan(current, appendNewLine: more ? false : appendNewLine);
            }

            StringBuilderCache.Release(sb);
        }

        public override void Write(string format, object arg0)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0), appendNewLine: false);
            }
            else
            {
                base.Write(format, arg0);
            }
        }

        public override void Write(string format, object arg0, object arg1)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0, arg1), appendNewLine: false);
            }
            else
            {
                base.Write(format, arg0, arg1);
            }
        }

        public override void Write(string format, object arg0, object arg1, object arg2)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0, arg1, arg2), appendNewLine: false);
            }
            else
            {
                base.Write(format, arg0, arg1, arg2);
            }
        }

        public override void Write(string format, params object[] arg)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg), appendNewLine: false);
            }
            else
            {
                base.Write(format, arg);
            }
        }

        public override void WriteLine(string format, object arg0)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0), appendNewLine: true);
            }
            else
            {
                base.WriteLine(format, arg0);
            }
        }

        public override void WriteLine(string format, object arg0, object arg1)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0, arg1), appendNewLine: true);
            }
            else
            {
                base.WriteLine(format, arg0, arg1);
            }
        }

        public override void WriteLine(string format, object arg0, object arg1, object arg2)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg0, arg1, arg2), appendNewLine: true);
            }
            else
            {
                base.WriteLine(format, arg0, arg1, arg2);
            }
        }

        public override void WriteLine(string format, params object[] arg)
        {
            if (GetType() == typeof(StreamWriter))
            {
                WriteFormatHelper(format, new ParamsArray(arg), appendNewLine: true);
            }
            else
            {
                base.WriteLine(format, arg);
            }
        }
    }  // class StreamWriter
}  // namespace
