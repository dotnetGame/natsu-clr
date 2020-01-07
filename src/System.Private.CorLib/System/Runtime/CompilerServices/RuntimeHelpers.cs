using System;
using System.Collections.Generic;
using System.Diagnostics;
using Internal.Runtime.CompilerServices;

namespace System.Runtime.CompilerServices
{
    public static partial class RuntimeHelpers
    {
        [Intrinsic]
        internal static ref byte GetRawSzArrayData(this Array array) =>
            ref Unsafe.As<RawSzArrayData>(array).Data;

        [Intrinsic]
        internal static ref char GetRawStringData(this string str) =>
            ref Unsafe.As<RawStringData>(str).Data;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int GetHashCode(object o);

        internal static ref byte GetRawData(this object obj) =>
            ref Unsafe.As<RawData>(obj).Data;

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool IsReferenceOrContainsReferences<TTo>() where TTo : struct;

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int EnumCompareTo<T>(T x, T y) where T : struct;

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool EnumEquals<T>(T x, T y) where T : struct;

        // Returns true iff the object has a component size;
        // i.e., is variable length like System.String or Array.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static unsafe bool ObjectHasComponentSize(object obj)
        {
            // CLR objects are laid out in memory as follows.
            // [ pMethodTable || .. object data .. ]
            //   ^-- the object reference points here
            //
            // The first DWORD of the method table class will have its high bit set if the
            // method table has component size info stored somewhere. See member
            // MethodTable:IsStringOrArray in src\vm\methodtable.h for full details.
            //
            // So in effect this method is the equivalent of
            // return ((MethodTable*)(*obj))->IsStringOrArray();

            Debug.Assert(obj != null);
            return *(int*)GetObjectMethodTablePointer(obj) < 0;
        }

        // Given an object reference, returns its MethodTable* as an IntPtr.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static IntPtr GetObjectMethodTablePointer(object obj)
        {
            Debug.Assert(obj != null);

            // We know that the first data field in any managed object is immediately after the
            // method table pointer, so just back up one pointer and immediately deref.
            // This is not ideal in terms of minimizing instruction count but is the best we can do at the moment.

            return Unsafe.Add(ref Unsafe.As<byte, IntPtr>(ref obj.GetRawData()), -1);

            // The JIT currently implements this as:
            // lea tmp, [rax + 8h] ; assume rax contains the object reference, tmp is type IntPtr&
            // mov tmp, qword ptr [tmp - 8h] ; tmp now contains the MethodTable* pointer
            //
            // Ideally this would just be a single dereference:
            // mov tmp, qword ptr [rax] ; rax = obj ref, tmp = MethodTable* pointer
        }

        public static int OffsetToStringData
        {
            // This offset is baked in by string indexer intrinsic, so there is no harm
            // in getting it baked in here as well.
            [System.Runtime.Versioning.NonVersionable]
            get
            {
                return 4;
            }
        }
    }
}
