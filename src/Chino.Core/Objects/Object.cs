using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    internal struct ObjectHeader
    {
        public string? Name;
        public Directory? Parent;
    }

    public enum ObjectParseStatus
    {
        Success,
        InvalidPath,
        NotFound
    }

    public struct ObjectAttributes
    {
        public static readonly ObjectAttributes Empty = new ObjectAttributes();

        public string? Name;
        public Accessor<Directory>? Root;
    }

    public abstract class Object
    {
        internal ObjectHeader _header;

        public string? Name => _header.Name;

        public virtual bool CanOpen => true;

        public virtual bool CanParse => false;

        public virtual AccessMask ValidAccessMask { get; } = AccessMask.Empty;

        public Object()
        {
        }

        protected internal virtual void Open(AccessMask grantedAccess)
        {
        }

        protected internal virtual ObjectParseStatus Parse(ref AccessState accessState, ref ReadOnlySpan<char> completeName, ref ReadOnlySpan<char> remainingName, out Object? foundObject)
        {
            throw new NotSupportedException();
        }
    }
}
