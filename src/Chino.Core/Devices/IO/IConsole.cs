using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Devices.IO
{
    public interface IConsole
    {
        event EventHandler DataAvailable;
    }
}
