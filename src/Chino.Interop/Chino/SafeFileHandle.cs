using System;
using System.Collections.Generic;
using System.Text;

namespace Chino
{
    public sealed class SafeFileHandle : IDisposable
    {
        private readonly IDisposable? _accessor;

        public bool IsInvalid => _accessor == null;

        public bool IsClosed { get; private set; }

        public void Dispose()
        {
            if (!IsClosed)
            {
                IsClosed = true;
                _accessor?.Dispose();
            }
        }
    }
}
