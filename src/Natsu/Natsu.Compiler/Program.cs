using System;
using System.Collections.Generic;
using dnlib.DotNet;

namespace Natsu.Compiler
{
    class Program
    {
        static void Main(string[] args)
        {
            var fileName = @"..\System.Private.CorLib\bin\Debug\netstandard2.0\System.Private.CorLib.dll";
            var module = ModuleDefMD.Load(fileName);
            var generator = new Generator(module);
            generator.Generate();
        }
    }

    class Generator
    {
        private readonly ModuleDefMD _module;

        public Generator(ModuleDefMD module)
        {
            _module = module;
        }

        public void Generate()
        {
            var moduleDesc = new ModuleDesc(_module);
            var namespaces = new List<NamespaceDesc>();
            NamespaceDesc GetOrAddNamespace(UTF8String name)
            {
                var desc = new NamespaceDesc
                {
                    Name = name,
                    QualifiedName =}
            }

            foreach (var type in _module.Types)
            {
                var ns = GetOrAddNamespace(type.Namespace);
            }
        }

        class ModuleDesc
        {
            public string Name { get; }

            public ModuleDesc(ModuleDefMD module)
            {
                Name = module.Name.Replace('.', '_');
            }
        }

        class NamespaceDesc
        {
            public string Name { get; }

            public string QualifiedName { get; }

            public Dictionary<UTF8String, NamespaceDesc> Nested { get; } = new Dictionary<UTF8String, NamespaceDesc>();

            public NamespaceDesc(ModuleDesc module, UTF8String name, NamespaceDesc parent = null)
            {
                if (name.Contains("."))
                    throw new ArgumentException("Namespace id is invalid: " + name);
                Name = name;

                if (parent != null)
                    QualifiedName = parent.QualifiedName + "::" + Name;
                else
                    QualifiedName = "::" + module.Name + Name;
            }
        }

        class TypeDesc
        {
            public UTF8String Name { get; set; }

            public string QualifiedName { get; set; }

            public Dictionary<UTF8String, TypeDesc> Nested { get; } = new Dictionary<UTF8String, TypeDesc>();
        }
    }
}
