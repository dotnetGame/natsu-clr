// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;

namespace System.IO
{
    /* SyncTextReader intentionally locks on itself rather than a private lock object.
     * This is done to synchronize different console readers (https://github.com/dotnet/corefx/issues/2855).
     */
    internal sealed partial class SyncTextReader : TextReader
    {
        internal readonly TextReader _in;

        public static SyncTextReader GetSynchronizedTextReader(TextReader reader)
        {
            Debug.Assert(reader != null);
            return reader as SyncTextReader ??
                new SyncTextReader(reader);
        }

        internal SyncTextReader(TextReader t)
        {
            _in = t;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                lock (this)
                {
                    _in.Dispose();
                }
            }
        }

        public override int Peek()
        {
            lock (this)
            {
                return _in.Peek();
            }
        }

        public override int Read()
        {
            lock (this)
            {
                return _in.Read();
            }
        }

        public override int Read(char[] buffer, int index, int count)
        {
            lock (this)
            {
                return _in.Read(buffer, index, count);
            }
        }

        public override int ReadBlock(char[] buffer, int index, int count)
        {
            lock (this)
            {
                return _in.ReadBlock(buffer, index, count);
            }
        }

        public override string? ReadLine()
        {
            lock (this)
            {
                return _in.ReadLine();
            }
        }

        public override string ReadToEnd()
        {
            lock (this)
            {
                return _in.ReadToEnd();
            }
        }
    }
}
