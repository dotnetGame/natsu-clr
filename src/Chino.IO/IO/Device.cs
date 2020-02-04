using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.IO
{
    public abstract class Device : Chino.Objects.Object
    {
        public abstract DeviceType DeviceType { get; }

        internal protected virtual void OnInstall()
        {
        }
    }
}
