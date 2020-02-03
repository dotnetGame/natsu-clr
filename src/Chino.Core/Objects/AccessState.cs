using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public struct AccessState
    {
        public AccessMask RemainingDesiredAccess;
        public AccessMask PreviouslyGrantedAccess;
        public AccessMask OriginalDesiredAccess;

        public AccessState(AccessMask desiredAccess)
        {
            RemainingDesiredAccess = desiredAccess;
            PreviouslyGrantedAccess = AccessMask.Empty;
            OriginalDesiredAccess = desiredAccess;
        }
    }
}
