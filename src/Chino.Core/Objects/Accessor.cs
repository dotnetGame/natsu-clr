using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public struct Accessor<T> : IEquatable<Accessor<T>> where T : Object
    {
        public T Object { get; }

        public AccessMask GrantedAccess { get; }

        internal Accessor(T @object, AccessMask grantedAccess)
        {
            Object = @object;
            GrantedAccess = grantedAccess;
        }

        public Accessor<U> Cast<U>() where U : Object
        {
            return new Accessor<U>((U)(object)Object, GrantedAccess);
        }

        public override bool Equals(object? obj)
        {
            return obj is Accessor<T> accessor && Equals(accessor);
        }

        public bool Equals(Accessor<T> other)
        {
            return EqualityComparer<T>.Default.Equals(Object, other.Object) &&
                   GrantedAccess.Equals(other.GrantedAccess);
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(Object, GrantedAccess);
        }

        public static bool operator ==(Accessor<T> left, Accessor<T> right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(Accessor<T> left, Accessor<T> right)
        {
            return !(left == right);
        }
    }
}
