using System;
using System.Collections.Generic;
using System.Text;

namespace Natsu
{
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = true)]
    public sealed class AssemblyEmbeddedCodeAttribute : Attribute
    {
        public string Code { get; }

        public AssemblyEmbeddedCodeAttribute(string code)
        {
            Code = code;
        }
    }
}
