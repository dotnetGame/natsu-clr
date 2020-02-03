using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public struct Accessor<T> where T : Object
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
    }
}
