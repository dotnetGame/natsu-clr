using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Text;

namespace Chino.Objects
{
    internal sealed class Directory : Object
    {
        private readonly Dictionary<string, Object> _children = new Dictionary<string, Object>();

        public Object this[string name]
        {
            get
            {
                lock (_children)
                {
                    return _children[name];
                }
            }
        }

        public void Add(Object node)
        {
            Debug.Assert(node._header.Name != null);
            lock (_children)
            {
                _children.Add(node._header.Name, node);
            }
        }

        public bool TryAdd(Object node)
        {
            Debug.Assert(node._header.Name != null);
            lock (_children)
            {
                return _children.TryAdd(node._header.Name, node);
            }
        }

        public bool TryGet(string name, [MaybeNullWhen(false)] out Object? node)
        {
            lock (_children)
            {
                return _children.TryGetValue(name, out node);
            }
        }

        public bool Remove(string name)
        {
            lock (_children)
            {
                return _children.Remove(name);
            }
        }
    }
}
