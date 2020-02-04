using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.IO
{
    public abstract class Driver : Chino.Objects.Object
    {
        internal protected virtual void OnInstall()
        {
        }

        internal protected abstract bool IsCompatible(DeviceDescription deviceDescription);

        internal protected abstract void InstallDevice(DeviceDescription deviceDescription);
    }
}
