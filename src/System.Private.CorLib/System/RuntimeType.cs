﻿using System;
using System.Collections.Generic;

namespace System
{
    internal class RuntimeType : Type
    {
        private readonly IntPtr _eeClass;

        internal RuntimeType(IntPtr eeClass)
        {
            _eeClass = eeClass;
        }
    }
}
