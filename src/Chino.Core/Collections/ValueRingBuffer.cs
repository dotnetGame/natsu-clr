using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;

namespace Chino.Collections
{
    public struct ValueRingBuffer<T>
    {
        private readonly T[] _buffer;
        private int _readPointer;
        private int _writePointer;

        public ValueRingBuffer(int capacity)
        {
            Debug.Assert(capacity > 0);
            _buffer = new T[capacity];
            _readPointer = 0;
            _writePointer = 0;
        }

        public int TryRead(Span<T> buffer)
        {
            Debug.Assert(buffer.Length > 0);
            int toRead = buffer.Length;
            if (toRead > 0)
            {
                var pointers = GetPointers();
                if (pointers.writePtr > pointers.readPtr)
                {
                    var avail = pointers.writePtr - pointers.readPtr;
                    var canRead = Math.Min(toRead, avail);
                    if (canRead > 0)
                    {
                        _buffer.AsSpan().Slice(pointers.readPtr, canRead).CopyTo(buffer);
                        Volatile.Write(ref _readPointer, pointers.readPtr + canRead);
                        return canRead;
                    }
                }
                else if (pointers.writePtr < pointers.readPtr)
                {
                    // read tail
                    var avail = _buffer.Length - pointers.readPtr;
                    var canRead = Math.Min(toRead, avail);
                    int read = 0;
                    if (canRead > 0)
                    {
                        _buffer.AsSpan().Slice(pointers.readPtr, canRead).CopyTo(buffer);
                        read += canRead;
                        toRead -= canRead;
                    }

                    avail = pointers.writePtr;
                    canRead = Math.Min(toRead, avail);
                    if (canRead > 0)
                    {
                        _buffer.AsSpan().Slice(0, canRead).CopyTo(buffer.Slice(read));
                        read += canRead;
                        Volatile.Write(ref _readPointer, canRead);
                    }
                    else
                    {
                        var pos = pointers.readPtr + read;
                        if (pos == _buffer.Length) pos = 0;
                        Volatile.Write(ref _readPointer, pos);
                    }

                    return read;
                }
            }

            return 0;
        }

        public int TryWrite(ReadOnlySpan<T> buffer)
        {
            throw new NotImplementedException();
            //Debug.Assert(buffer.Length > 0);
            //int toWrite = buffer.Length;
            //if (toWrite > 0)
            //{
            //    var pointers = GetPointers();
            //    if (pointers.writePtr < pointers.readPtr)
            //    {
            //        var avail = pointers.writePtr - pointers.readPtr;
            //        var canRead = Math.Min(toWrite, avail);
            //        if (canRead > 0)
            //        {
            //            _buffer.AsSpan().Slice(pointers.readPtr, canRead).CopyTo(buffer);
            //            Volatile.Write(ref _readPointer, pointers.readPtr + canRead);
            //            return canRead;
            //        }
            //    }
            //    else if (pointers.writePtr < pointers.readPtr)
            //    {
            //        // read tail
            //        var avail = _buffer.Length - pointers.readPtr;
            //        var canRead = Math.Min(toWrite, avail);
            //        int read = 0;
            //        if (canRead > 0)
            //        {
            //            _buffer.AsSpan().Slice(pointers.readPtr, canRead).CopyTo(buffer);
            //            read += canRead;
            //            toWrite -= canRead;
            //        }

            //        avail = pointers.writePtr;
            //        canRead = Math.Min(toWrite, avail);
            //        if (canRead > 0)
            //        {
            //            _buffer.AsSpan().Slice(0, canRead).CopyTo(buffer.Slice(read));
            //            read += canRead;
            //            Volatile.Write(ref _readPointer, canRead);
            //        }
            //        else
            //        {
            //            var pos = pointers.readPtr + read;
            //            if (pos == _buffer.Length) pos = 0;
            //            Volatile.Write(ref _readPointer, pos);
            //        }

            //        return read;
            //    }
            //}

            //return 0;
        }

        private (int readPtr, int writePtr) GetPointers()
        {
            return (Volatile.Read(ref _readPointer), Volatile.Read(ref _writePointer));
        }
    }
}
