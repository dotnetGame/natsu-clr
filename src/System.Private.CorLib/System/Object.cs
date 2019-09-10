using System;
using System.Runtime.CompilerServices;

namespace System
{
    public class Object
    {
        public Object()
        {

        }

        // Allow an object to free resources before the object is reclaimed by the GC.
        // 
        [System.Runtime.Versioning.NonVersionable]
        ~Object()
        {
        }

        // Returns a String which represents the object instance.  The default
        // for an object is to return the fully qualified name of the class.
        // 
        public virtual string ToString()
        {
            return GetType().ToString();
        }

        // Returns a boolean indicating if the passed in object obj is 
        // Equal to this.  Equality is defined as object equality for reference
        // types and bitwise equality for value types using a loader trick to
        // replace Equals with EqualsValue for value types).
        // 

        public virtual bool Equals(object obj)
        {
            return ReferenceEquals(this, obj);
        }

        public static bool Equals(object objA, object objB)
        {
            if (objA == objB)
            {
                return true;
            }
            if (objA == null || objB == null)
            {
                return false;
            }
            return objA.Equals(objB);
        }

        [System.Runtime.Versioning.NonVersionable]
        public static bool ReferenceEquals(object objA, object objB)
        {
            return objA == objB;
        }

        // GetHashCode is intended to serve as a hash function for this object.
        // Based on the contents of the object, the hash function will return a suitable
        // value with a relatively random distribution over the various inputs.
        //
        // The default implementation returns the sync block index for this instance.
        // Calling it on the same object multiple times will return the same value, so
        // it will technically meet the needs of a hash function, but it's less than ideal.
        // Objects (& especially value classes) should override this method.
        // 
        public virtual int GetHashCode()
        {
            return 0;
        }

        // Returns a Type object which represent this object instance.
        // 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern Type GetType();

        public object MemberwiseClone()
        {
            throw new NotImplementedException();
        }
    }
}
