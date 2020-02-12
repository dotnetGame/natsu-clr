using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Internal.Runtime.CompilerServices;
#if BIT64
using nuint = System.UInt64;
#else
using nuint = System.UInt32;
#endif

namespace System
{
    public class Array
    {
        // We impose limits on maximum array lenght in each dimension to allow efficient 
        // implementation of advanced range check elimination in future.
        // Keep in sync with vm\gcscan.cpp and HashHelpers.MaxPrimeArrayLength.
        // The constants are defined in this method: inline SIZE_T MaxArrayLength(SIZE_T componentSize) from gcscan
        // We have different max sizes for arrays with elements of size 1 for backwards compatibility
        internal const int MaxArrayLength = 0X7FEFFFFF;
        internal const int MaxByteArrayLength = 0x7FFFFFC7;

        public extern long LongLength
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetLength(int dimension);

        public long GetLongLength(int dimension)
        {
            //This method should throw an IndexOufOfRangeException for compat if dimension < 0 or >= Rank
            return GetLength(dimension);
        }

        public extern int Rank
        {
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            get;
        }

        public extern int Length
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetUpperBound(int dimension);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetLowerBound(int dimension);

        // CopyTo copies a collection into an Array, starting at a particular
        // index into the array.
        // 
        // This method is to support the ICollection interface, and calls
        // Array.Copy internally.  If you aren't using ICollection explicitly,
        // call Array.Copy to avoid an extra indirection.
        // 
        public void CopyTo(Array array, int index)
        {
            if (array != null && array.Rank != 1)
                ThrowHelper.ThrowArgumentException(ExceptionResource.Arg_RankMultiDimNotSupported);
            // Note: Array.Copy throws a RankException and we want a consistent ArgumentException for all the IList CopyTo methods.
            Array.Copy(this, GetLowerBound(0), array, index, Length);
        }

        public void CopyTo(Array array, long index)
        {
            if (index > int.MaxValue || index < int.MinValue)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.index, ExceptionResource.ArgumentOutOfRange_HugeArrayNotSupported);

            this.CopyTo(array, (int)index);
        }

        private static class EmptyArray<T>
        {
            internal static readonly T[] Value = new T[0];
        }

        public static T[] Empty<T>()
        {
            return EmptyArray<T>.Value;
        }

        // Copies length elements from sourceArray, starting at index 0, to
        // destinationArray, starting at index 0.
        //
        public static void Copy(Array sourceArray, Array destinationArray, int length)
        {
            if (sourceArray == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.sourceArray);
            if (destinationArray == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.destinationArray);

            Copy(sourceArray, sourceArray.GetLowerBound(0), destinationArray, destinationArray.GetLowerBound(0), length, false);
        }

        // Copies length elements from sourceArray, starting at sourceIndex, to
        // destinationArray, starting at destinationIndex.
        //
        public static void Copy(Array sourceArray, int sourceIndex, Array destinationArray, int destinationIndex, int length)
        {
            Copy(sourceArray, sourceIndex, destinationArray, destinationIndex, length, false);
        }

        // Reliability-wise, this method will either possibly corrupt your 
        // instance & might fail when called from within a CER, or if the
        // reliable flag is true, it will either always succeed or always
        // throw an exception with no side effects.
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void Copy(Array sourceArray, int sourceIndex, Array destinationArray, int destinationIndex, int length, bool reliable);

        // Provides a strong exception guarantee - either it succeeds, or
        // it throws an exception with no side effects.  The arrays must be
        // compatible array types based on the array element type - this 
        // method does not support casting, boxing, or primitive widening.
        // It will up-cast, assuming the array types are correct.
        public static void ConstrainedCopy(Array sourceArray, int sourceIndex, Array destinationArray, int destinationIndex, int length)
        {
            Copy(sourceArray, sourceIndex, destinationArray, destinationIndex, length, true);
        }

        public static void Copy(Array sourceArray, Array destinationArray, long length)
        {
            if (length > int.MaxValue || length < int.MinValue)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.length, ExceptionResource.ArgumentOutOfRange_HugeArrayNotSupported);

            Array.Copy(sourceArray, destinationArray, (int)length);
        }

        public static void Copy(Array sourceArray, long sourceIndex, Array destinationArray, long destinationIndex, long length)
        {
            if (sourceIndex > int.MaxValue || sourceIndex < int.MinValue)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.sourceIndex, ExceptionResource.ArgumentOutOfRange_HugeArrayNotSupported);
            if (destinationIndex > int.MaxValue || destinationIndex < int.MinValue)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.destinationIndex, ExceptionResource.ArgumentOutOfRange_HugeArrayNotSupported);
            if (length > int.MaxValue || length < int.MinValue)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.length, ExceptionResource.ArgumentOutOfRange_HugeArrayNotSupported);

            Array.Copy(sourceArray, (int)sourceIndex, destinationArray, (int)destinationIndex, (int)length);
        }

        public static void Resize<T>(ref T[] array, int newSize)
        {
            if (newSize < 0)
                ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.newSize, ExceptionResource.ArgumentOutOfRange_NeedNonNegNum);

            T[] larray = array;
            if (larray == null)
            {
                array = new T[newSize];
                return;
            }

            if (larray.Length != newSize)
            {
                T[] newArray = new T[newSize];
                Array.Copy(larray, 0, newArray, 0, larray.Length > newSize ? newSize : larray.Length);
                array = newArray;
            }
        }

        // Make a new array which is a shallow copy of the original array.
        // 
        public object Clone()
        {
            return MemberwiseClone();
        }

        public static int BinarySearch<T>(T[] array, T value)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            return BinarySearch<T>(array, 0, array.Length, value, null);
        }

        public static int BinarySearch<T>(T[] array, T value, System.Collections.Generic.IComparer<T>? comparer)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            return BinarySearch<T>(array, 0, array.Length, value, comparer);
        }

        public static int BinarySearch<T>(T[] array, int index, int length, T value)
        {
            return BinarySearch<T>(array, index, length, value, null);
        }

        public static int BinarySearch<T>(T[] array, int index, int length, T value, System.Collections.Generic.IComparer<T>? comparer)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            if (index < 0)
                ThrowHelper.ThrowIndexArgumentOutOfRange_NeedNonNegNumException();
            if (length < 0)
                ThrowHelper.ThrowLengthArgumentOutOfRange_ArgumentOutOfRange_NeedNonNegNum();

            if (array.Length - index < length)
                ThrowHelper.ThrowArgumentException(ExceptionResource.Argument_InvalidOffLen);

            return ArraySortHelper<T>.Default.BinarySearch(array, index, length, value, comparer);
        }

        public static int IndexOf<T>(T[] array, T value)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }

            return IndexOf(array, value, 0, array.Length);
        }

        public static int IndexOf<T>(T[] array, T value, int startIndex)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }

            return IndexOf(array, value, startIndex, array.Length - startIndex);
        }

        public static int IndexOf<T>(T[] array, T value, int startIndex, int count)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }

            if ((uint)startIndex > (uint)array.Length)
            {
                ThrowHelper.ThrowStartIndexArgumentOutOfRange_ArgumentOutOfRange_Index();
            }

            if ((uint)count > (uint)(array.Length - startIndex))
            {
                ThrowHelper.ThrowCountArgumentOutOfRange_ArgumentOutOfRange_Count();
            }

            if (RuntimeHelpers.IsBitwiseEquatable<T>())
            {
                if (Unsafe.SizeOf<T>() == sizeof(byte))
                {
                    int result = SpanHelpers.IndexOf(
                        ref Unsafe.Add(ref array.GetRawSzArrayData(), startIndex),
                        Unsafe.As<T, byte>(ref value),
                        count);
                    return (result >= 0 ? startIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(char))
                {
                    int result = SpanHelpers.IndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, char>(ref array.GetRawSzArrayData()), startIndex),
                        Unsafe.As<T, char>(ref value),
                        count);
                    return (result >= 0 ? startIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(int))
                {
                    int result = SpanHelpers.IndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, int>(ref array.GetRawSzArrayData()), startIndex),
                        Unsafe.As<T, int>(ref value),
                        count);
                    return (result >= 0 ? startIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(long))
                {
                    int result = SpanHelpers.IndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, long>(ref array.GetRawSzArrayData()), startIndex),
                        Unsafe.As<T, long>(ref value),
                        count);
                    return (result >= 0 ? startIndex : 0) + result;
                }
            }

            return EqualityComparer<T>.Default.IndexOf(array, value, startIndex, count);
        }

        public static void Sort<T>(T[] array)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            Sort<T>(array, 0, array.Length, null);
        }

        public static void Sort<T>(T[] array, System.Collections.Generic.IComparer<T>? comparer)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            Sort<T>(array, 0, array.Length, comparer);
        }

        public static void Sort<T>(T[] array, int index, int length, System.Collections.Generic.IComparer<T>? comparer)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            if (index < 0)
                ThrowHelper.ThrowIndexArgumentOutOfRange_NeedNonNegNumException();
            if (length < 0)
                ThrowHelper.ThrowLengthArgumentOutOfRange_ArgumentOutOfRange_NeedNonNegNum();
            if (array.Length - index < length)
                ThrowHelper.ThrowArgumentException(ExceptionResource.Argument_InvalidOffLen);

            if (length > 1)
            {
#if CORECLR
                if (comparer == null || comparer == Comparer<T>.Default)
                {
                    if (TrySZSort(array, null, index, index + length - 1))
                    {
                        return;
                    }
                }
#endif

                ArraySortHelper<T>.Default.Sort(array, index, length, comparer);
            }
        }

        // Sets length elements in array to 0 (or null for Object arrays), starting
        // at index.
        //
        public static unsafe void Clear(Array array, int index, int length)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);

            ref byte p = ref GetRawArrayGeometry(array, out uint numComponents, out uint elementSize, out int lowerBound, out bool containsGCPointers);

            int offset = index - lowerBound;

            if (index < lowerBound || offset < 0 || length < 0 || (uint)(offset + length) > numComponents)
                ThrowHelper.ThrowIndexOutOfRangeException();

            ref byte ptr = ref Unsafe.AddByteOffset(ref p, (uint)offset * (nuint)elementSize);
            nuint byteLength = (uint)length * (nuint)elementSize;

            if (containsGCPointers)
                SpanHelpers.ClearWithReferences(ref Unsafe.As<byte, IntPtr>(ref ptr), byteLength / (uint)sizeof(IntPtr));
            else
                SpanHelpers.ClearWithoutReferences(ref ptr, byteLength);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ref byte GetRawArrayGeometry(Array array, out uint numComponents, out uint elementSize, out int lowerBound, out bool containsGCPointers);


        public static int LastIndexOf<T>(T[] array, T value)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }

            return LastIndexOf(array, value, array.Length - 1, array.Length);
        }

        public static int LastIndexOf<T>(T[] array, T value, int startIndex)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }
            // if array is empty and startIndex is 0, we need to pass 0 as count
            return LastIndexOf(array, value, startIndex, (array.Length == 0) ? 0 : (startIndex + 1));
        }

        public static int LastIndexOf<T>(T[] array, T value, int startIndex, int count)
        {
            if (array == null)
            {
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            }

            if (array.Length == 0)
            {
                //
                // Special case for 0 length List
                // accept -1 and 0 as valid startIndex for compablility reason.
                //
                if (startIndex != -1 && startIndex != 0)
                {
                    ThrowHelper.ThrowStartIndexArgumentOutOfRange_ArgumentOutOfRange_Index();
                }

                // only 0 is a valid value for count if array is empty
                if (count != 0)
                {
                    ThrowHelper.ThrowCountArgumentOutOfRange_ArgumentOutOfRange_Count();
                }
                return -1;
            }

            // Make sure we're not out of range
            if ((uint)startIndex >= (uint)array.Length)
            {
                ThrowHelper.ThrowStartIndexArgumentOutOfRange_ArgumentOutOfRange_Index();
            }

            // 2nd have of this also catches when startIndex == MAXINT, so MAXINT - 0 + 1 == -1, which is < 0.
            if (count < 0 || startIndex - count + 1 < 0)
            {
                ThrowHelper.ThrowCountArgumentOutOfRange_ArgumentOutOfRange_Count();
            }

            if (RuntimeHelpers.IsBitwiseEquatable<T>())
            {
                if (Unsafe.SizeOf<T>() == sizeof(byte))
                {
                    int endIndex = startIndex - count + 1;
                    int result = SpanHelpers.LastIndexOf(
                        ref Unsafe.Add(ref array.GetRawSzArrayData(), endIndex),
                        Unsafe.As<T, byte>(ref value),
                        count);

                    return (result >= 0 ? endIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(char))
                {
                    int endIndex = startIndex - count + 1;
                    int result = SpanHelpers.LastIndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, char>(ref array.GetRawSzArrayData()), endIndex),
                        Unsafe.As<T, char>(ref value),
                        count);

                    return (result >= 0 ? endIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(int))
                {
                    int endIndex = startIndex - count + 1;
                    int result = SpanHelpers.LastIndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, int>(ref array.GetRawSzArrayData()), endIndex),
                        Unsafe.As<T, int>(ref value),
                        count);

                    return (result >= 0 ? endIndex : 0) + result;
                }
                else if (Unsafe.SizeOf<T>() == sizeof(long))
                {
                    int endIndex = startIndex - count + 1;
                    int result = SpanHelpers.LastIndexOf(
                        ref Unsafe.Add(ref Unsafe.As<byte, long>(ref array.GetRawSzArrayData()), endIndex),
                        Unsafe.As<T, long>(ref value),
                        count);

                    return (result >= 0 ? endIndex : 0) + result;
                }
            }

            return EqualityComparer<T>.Default.LastIndexOf(array, value, startIndex, count);
        }

        public static void Reverse<T>(T[] array)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            Reverse(array, 0, array.Length);
        }

        public static void Reverse<T>(T[] array, int index, int length)
        {
            if (array == null)
                ThrowHelper.ThrowArgumentNullException(ExceptionArgument.array);
            if (index < 0)
                ThrowHelper.ThrowIndexArgumentOutOfRange_NeedNonNegNumException();
            if (length < 0)
                ThrowHelper.ThrowLengthArgumentOutOfRange_ArgumentOutOfRange_NeedNonNegNum();
            if (array.Length - index < length)
                ThrowHelper.ThrowArgumentException(ExceptionResource.Argument_InvalidOffLen);

            if (length <= 1)
                return;

            ref T first = ref Unsafe.Add(ref Unsafe.As<byte, T>(ref array.GetRawSzArrayData()), index);
            ref T last = ref Unsafe.Add(ref Unsafe.Add(ref first, length), -1);
            do
            {
                T temp = first;
                first = last;
                last = temp;
                first = ref Unsafe.Add(ref first, 1);
                last = ref Unsafe.Add(ref last, -1);
            } while (Unsafe.IsAddressLessThan(ref first, ref last));
        }
    }
}
