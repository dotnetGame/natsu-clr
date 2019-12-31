using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace System
{
    public abstract class Delegate : ICloneable
    {
        // _target is the object we will invoke on
        internal object _target;

        // _methodPtr is a pointer to the method we will invoke
        // It could be a small thunk if this is a static or UM call
        internal IntPtr _methodPtr;

        // Protect the default constructor so you can't build a delegate
        internal Delegate()
        {
        }

        public virtual object Clone() => MemberwiseClone();

        [return: NotNullIfNotNull("a")]
        [return: NotNullIfNotNull("b")]
        public static Delegate? Combine(Delegate? a, Delegate? b)
        {
            if (a is null)
                return b;
            if (b is null)
                return a;

            return a.CombineImpl(b);
        }

        public static Delegate? Combine(params Delegate?[]? delegates)
        {
            if (delegates == null || delegates.Length == 0)
                return null;

            Delegate? d = delegates[0];
            for (int i = 1; i < delegates.Length; i++)
                d = Combine(d, delegates[i]);

            return d;
        }

        protected virtual Delegate CombineImpl(Delegate d) => throw new MulticastNotSupportedException(SR.Multicast_Combine);

        protected virtual Delegate? RemoveImpl(Delegate d) => d.Equals(this) ? null : this;

        public virtual Delegate[] GetInvocationList() => new Delegate[] { this };

        public static Delegate? Remove(Delegate? source, Delegate? value)
        {
            if (source == null)
                return null;

            if (value == null)
                return source;

            return source.RemoveImpl(value);
        }

        public static Delegate? RemoveAll(Delegate? source, Delegate? value)
        {
            Delegate? newDelegate;

            do
            {
                newDelegate = source;
                source = Remove(source, value);
            }
            while (newDelegate != source);

            return newDelegate;
        }

        // Force inline as the true/false ternary takes it above ALWAYS_INLINE size even though the asm ends up smaller
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Delegate? d1, Delegate? d2)
        {
            // Test d2 first to allow branch elimination when inlined for null checks (== null)
            // so it can become a simple test
            if (d2 is null)
            {
                // return true/false not the test result https://github.com/dotnet/coreclr/issues/914
                return (d1 is null) ? true : false;
            }

            return ReferenceEquals(d2, d1) ? true : d2.Equals((object?)d1);
        }

        // Force inline as the true/false ternary takes it above ALWAYS_INLINE size even though the asm ends up smaller
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(Delegate? d1, Delegate? d2)
        {
            // Test d2 first to allow branch elimination when inlined for not null checks (!= null)
            // so it can become a simple test
            if (d2 is null)
            {
                // return true/false not the test result https://github.com/dotnet/coreclr/issues/914
                return (d1 is null) ? false : true;
            }

            return ReferenceEquals(d2, d1) ? false : !d2.Equals(d1);
        }
    }
}
