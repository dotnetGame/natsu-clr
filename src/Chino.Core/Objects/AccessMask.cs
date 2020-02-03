using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public struct AccessMask : IEquatable<AccessMask>
    {
        public static readonly AccessMask Empty = default;
        public static readonly AccessMask GenericRead = 1 << 0;
        public static readonly AccessMask GenericWrite = 1 << 1;
        public static readonly AccessMask GenericExecute = 1 << 2;
        public static readonly AccessMask GenericAll = 1 << 3;

        public uint Value { get; set; }

        public static implicit operator AccessMask(uint value)
        {
            return new AccessMask { Value = value };
        }

        public static AccessMask operator &(AccessMask lhs, AccessMask rhs)
        {
            return new AccessMask { Value = lhs.Value & rhs.Value };
        }

        public static AccessMask operator |(AccessMask lhs, AccessMask rhs)
        {
            return new AccessMask { Value = lhs.Value | rhs.Value };
        }

        public static bool operator ==(AccessMask left, AccessMask right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(AccessMask left, AccessMask right)
        {
            return !(left == right);
        }

        public void CheckAccess(AccessMask requiredAccess)
        {
            if ((this & requiredAccess) != requiredAccess)
                throw new UnauthorizedAccessException();
        }

        public override bool Equals(object? obj)
        {
            return obj is AccessMask mask && Equals(mask);
        }

        public bool Equals(AccessMask other)
        {
            return Value == other.Value;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(Value);
        }
    }
}
