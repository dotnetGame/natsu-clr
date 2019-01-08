

namespace System
{
    // Note: users should make sure they copy the fields out of an ArraySegment onto their stack
    // then validate that the fields describe valid bounds within the array.  This must be done
    // because assignments to value types are not atomic, and also because one thread reading 
    // three fields from an ArraySegment may not see the same ArraySegment from one call to another
    // (ie, users could assign a new value to the old location).  
    [Serializable]
    public readonly struct ArraySegment<T>
    {
        public T[] Array { get; }

        public int Offset { get; }

        public int Count { get; }

        public ArraySegment(T[] array)
            :this(array, 0, array.Length)
        {
        }

        public ArraySegment(T[] array, int offset, int count)
        {
            Array = array;
            Offset = offset;
            Count = count;
        }
    }
}
