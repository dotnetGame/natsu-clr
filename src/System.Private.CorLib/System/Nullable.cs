// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.Versioning;

namespace System
{
    // Because we have special type system support that says a a boxed Nullable<T>
    // can be used where a boxed<T> is use, Nullable<T> can not implement any intefaces
    // at all (since T may not).   Do NOT add any interfaces to Nullable!
    //
    [Serializable]
    [NonVersionable] // This only applies to field layout
    public struct Nullable<T> where T : struct
    {
        private readonly bool hasValue; // Do not rename (binary serialization)
        internal T value; // Do not rename (binary serialization) or make readonly (can be mutated in ToString, etc.)

        [NonVersionable]
        public Nullable(T value)
        {
            this.value = value;
            hasValue = true;
        }

        public bool HasValue
        {
            [NonVersionable]
            get
            {
                return hasValue;
            }
        }

        public T Value
        {
            get
            {
                return value;
            }
        }

        [NonVersionable]
        public T GetValueOrDefault()
        {
            return value;
        }

        [NonVersionable]
        public T GetValueOrDefault(T defaultValue)
        {
            return hasValue ? value : defaultValue;
        }

        public override bool Equals(object other)
        {
            if (!hasValue) return other == null;
            if (other == null) return false;
            return value.Equals(other);
        }

        public override int GetHashCode()
        {
            return hasValue ? value.GetHashCode() : 0;
        }

        public override string ToString()
        {
            return hasValue ? value.ToString() : "";
        }

        [NonVersionable]
        public static implicit operator Nullable<T>(T value)
        {
            return new Nullable<T>(value);
        }

        [NonVersionable]
        public static explicit operator T(Nullable<T> value)
        {
            return value.Value;
        }
    }

    public static class Nullable
    {
    }
}
