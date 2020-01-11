namespace System
{
    internal static partial class SR
    {
        public static string Arg_NonZeroLowerBound => @"The lower bound of target array must be zero.";
        public static string Arg_WrongType => @"The value '{0}' is not of type '{1}' and cannot be used in this generic collection.";
        public static string Arg_ArrayPlusOffTooSmall => @"Destination array is not long enough to copy all the items in the collection. Check array index and length.";
        public static string ArgumentOutOfRange_NeedNonNegNum => @"Non-negative number required.";
        public static string ArgumentOutOfRange_SmallCapacity => @"capacity was less than the current size.";
        public static string Argument_InvalidOffLen => @"Offset and length were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.";
        public static string Argument_AddingDuplicate => @"An item with the same key has already been added. Key: {0}";
        public static string InvalidOperation_ConcurrentOperationsNotSupported => @"Operations that change non-concurrent collections must have exclusive access. A concurrent update was performed on this collection and corrupted its state. The collection's state is no longer correct.";
        public static string InvalidOperation_EmptyQueue => @"Queue empty.";
        public static string InvalidOperation_EnumOpCantHappen => @"Enumeration has either not started or has already finished.";
        public static string InvalidOperation_EnumFailedVersion => @"Collection was modified; enumeration operation may not execute.";
        public static string InvalidOperation_EmptyStack => @"Stack empty.";
        public static string InvalidOperation_EnumNotStarted => @"Enumeration has not started. Call MoveNext.";
        public static string InvalidOperation_EnumEnded => @"Enumeration already finished.";
        public static string NotSupported_KeyCollectionSet => @"Mutating a key collection derived from a dictionary is not allowed.";
        public static string NotSupported_ValueCollectionSet => @"Mutating a value collection derived from a dictionary is not allowed.";
        public static string Arg_ArrayLengthsDiffer => @"Array lengths must be the same.";
        public static string Arg_BitArrayTypeUnsupported => @"Only supported array types for CopyTo on BitArrays are Boolean[], Int32[] and Byte[].";
        public static string Arg_HSCapacityOverflow => @"HashSet capacity is too big.";
        public static string Arg_HTCapacityOverflow => @"Hashtable's capacity overflowed and went negative. Check load factor, capacity and the current size of the table.";
        public static string Arg_InsufficientSpace => @"Insufficient space in the target location to copy the information.";
        public static string Arg_RankMultiDimNotSupported => @"Only single dimensional arrays are supported for the requested action.";
        public static string Argument_ArrayTooLarge => @"The input array length must not exceed Int32.MaxValue / {0}. Otherwise BitArray.Length would exceed Int32.MaxValue.";
        public static string Argument_InvalidArrayType => @"Target array type is not compatible with the type of items in the collection.";
        public static string ArgumentOutOfRange_BiggerThanCollection => @"Must be less than or equal to the size of the collection.";
        public static string ArgumentOutOfRange_Index => @"Index was out of range. Must be non-negative and less than the size of the collection.";
        public static string ExternalLinkedListNode => @"The LinkedList node does not belong to current LinkedList.";
        public static string LinkedListEmpty => @"The LinkedList is empty.";
        public static string LinkedListNodeIsAttached => @"The LinkedList node already belongs to a LinkedList.";
        public static string NotSupported_SortedListNestedWrite => @"This operation is not supported on SortedList nested types because they require modifying the original SortedList.";
        public static string SortedSet_LowerValueGreaterThanUpperValue => @"Must be less than or equal to upperValue.";
        public static string Serialization_InvalidOnDeser => @"OnDeserialization method was called while the object was not being deserialized.";
        public static string Serialization_MismatchedCount => @"The serialized Count information doesn't match the number of items.";
        public static string Serialization_MissingKeys => @"The keys for this dictionary are missing.";
        public static string Serialization_MissingValues => @"The values for this dictionary are missing.";
        public static string Arg_KeyNotFoundWithKey => @"The given key '{0}' was not present in the dictionary.";
    }
}
