using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public struct ObjectHeader
    {
        public string Name;
    }

    public enum ObjectParseStatus
    {
        Success,
        InvalidPath,
        NotFound
    }

    public class Object
    {
        internal ObjectHeader _header;

        public virtual bool CanParse => false;

        public virtual ObjectParseStatus Parse(ref ReadOnlySpan<char> completeName, ref ReadOnlySpan<char> remainingName, out Object? foundObject)
        {
            throw new NotSupportedException();
        }
    }
}
