using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Collections
{
    public class SListEntry
    {
        internal SListEntry _next;
    }

    public class ConcurrentSList
    {
        private volatile SListEntry? _head;
        private volatile SListEntry? _tail;

        public bool IsEmpty => _head == null;

        public int Count
        {
            get
            {
                int count = 0;
                for (SListEntry? cnt = _head; cnt != null; cnt = cnt._next)
                    count++;
                return count;
            }
        }

        public void Clear()
        {
            _head = null;
        }
    }
}
