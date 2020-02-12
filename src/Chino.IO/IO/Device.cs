using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.IO
{
    public abstract class Device : Chino.Objects.Object
    {
        public abstract DeviceType DeviceType { get; }

        public virtual bool CanRead => false;

        public virtual bool CanWrite => false;

        internal protected virtual void OnInstall()
        {
        }

        internal protected virtual int Read(Span<byte> buffer)
        {
            throw new NotSupportedException();
        }

        internal protected virtual void Write(ReadOnlySpan<byte> buffer)
        {
            throw new NotSupportedException();
        }
    }
}
