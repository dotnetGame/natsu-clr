using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Text;

namespace Chino.Objects
{
    public sealed class Directory : Object
    {
        private readonly Dictionary<string, Object> _children = new Dictionary<string, Object>();

        public override bool CanOpen => true;

        public override bool CanParse => true;

        internal Directory()
        {
        }

        internal void Add(Object node)
        {
            ObjectManager.ValidateObjectName(node._header.Name);
            lock (_children)
            {
                _children.Add(node._header.Name!, node);
            }
        }

        internal bool TryAdd(Object node)
        {
            ObjectManager.ValidateObjectName(node._header.Name);
            lock (_children)
            {
                return _children.TryAdd(node._header.Name!, node);
            }
        }

        internal bool TryGet(ReadOnlySpan<char> name, [MaybeNullWhen(false)] out Object node)
        {
            lock (_children)
            {
                // TODO: Remove ToString
                return _children.TryGetValue(name.ToString(), out node);
            }
        }

        internal bool Remove(ReadOnlySpan<char> name)
        {
            lock (_children)
            {
                // TODO: Remove ToString
                return _children.Remove(name.ToString());
            }
        }

        protected internal override void Open(AccessMask grantedAccess)
        {
        }

        protected internal override ObjectParseStatus Parse(ref AccessState accessState, ref ReadOnlySpan<char> completeName, ref ReadOnlySpan<char> remainingName, out Object? foundObject)
        {
            if (remainingName.IsEmpty)
            {
                foundObject = this;
                return ObjectParseStatus.Success;
            }

            var separatorIndex = remainingName.IndexOf(ObjectManager.DirectorySeparatorChar);
            ReadOnlySpan<char> componentName;
            if (separatorIndex != -1)
            {
                componentName = remainingName.Slice(0, separatorIndex);
                remainingName = remainingName.Slice(separatorIndex + 1);
            }
            else
            {
                componentName = remainingName;
                remainingName = ReadOnlySpan<char>.Empty;
            }

            if (TryGet(componentName, out var componentObject))
            {
                if (remainingName.IsEmpty)
                {
                    foundObject = componentObject;
                    return ObjectParseStatus.Success;
                }
                else
                {
                    if (componentObject.CanParse)
                    {
                        return componentObject.Parse(ref accessState, ref completeName, ref remainingName, out foundObject);
                    }
                    else
                    {
                        foundObject = null;
                        return ObjectParseStatus.InvalidPath;
                    }
                }
            }
            else
            {
                foundObject = null;
                return ObjectParseStatus.NotFound;
            }
        }
    }
}
