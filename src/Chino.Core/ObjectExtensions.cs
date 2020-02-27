using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;

namespace Chino
{
    public static class ObjectExtensions
    {
        public static DirectoryEntryInfo[] GetChildren(this IAccessor<Directory> directory)
        {
            return directory.Object.GetChildren();
        }
    }
}
