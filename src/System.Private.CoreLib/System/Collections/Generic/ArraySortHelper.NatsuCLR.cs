// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;

namespace System.Collections.Generic
{
    internal interface IArraySortHelper<TKey>
    {
        void Sort(TKey[] keys, int index, int length, IComparer<TKey>? comparer);
        int BinarySearch(TKey[] keys, int index, int length, TKey value, IComparer<TKey>? comparer);
    }

    [TypeDependencyAttribute("System.Collections.Generic.GenericArraySortHelper`1")]
    internal partial class ArraySortHelper<T>
        : IArraySortHelper<T>
    {
        private static readonly IArraySortHelper<T> s_defaultArraySortHelper = CreateArraySortHelper();

        public static IArraySortHelper<T> Default => s_defaultArraySortHelper;

        private static IArraySortHelper<T> CreateArraySortHelper()
        {
            IArraySortHelper<T> defaultArraySortHelper;

            if (RuntimeTypeHelpers.IsAssignableFrom<T, IComparable<T>>())
            {
                defaultArraySortHelper = RuntimeTypeHelpers.AllocateLike<GenericArraySortHelper<string>, IArraySortHelper<T>, T>();
            }
            else
            {
                defaultArraySortHelper = new ArraySortHelper<T>();
            }
            return defaultArraySortHelper;
        }
    }

    internal partial class GenericArraySortHelper<T>
        : IArraySortHelper<T>
    {
    }
}
