﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Runtime.CompilerServices;
using System.Text;
using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace Natsu.Compiler
{
    class Program
    {
        static void Main(string[] args)
        {
            var fileName = @"..\..\..\..\System.Private.CorLib\bin\Debug\netcoreapp3.0\System.Private.CorLib.dll";
            var module = ModuleDefMD.Load(fileName);
            var generator = new Generator(module);
            generator.Generate();
        }
    }

    class Generator
    {
        private readonly ModuleDefMD _module;
        private readonly Dictionary<TypeDef, TypeDesc> _typeDescs = new Dictionary<TypeDef, TypeDesc>();
        private readonly List<TypeDesc> _sortedTypeDescs = new List<TypeDesc>();

        public Generator(ModuleDefMD module)
        {
            _module = module;
        }

        public void Generate()
        {
            foreach (var type in _module.GetTypes())
            {
                var typeDesc = new TypeDesc(type);
                _typeDescs.Add(type, typeDesc);
                if (type.NestedTypes.Count != 0)
                    throw new NotImplementedException();
            }

            SortTypes();

            using (var writer = new StreamWriter($"{_module.Assembly.Name}.h"))
            {
                writer.WriteLine("// Generated by natsu clr compiler.");
                writer.WriteLine("#pragma once");
                writer.WriteLine("#include <natsu.runtime.h>");
                writer.WriteLine();

                writer.WriteLine($"namespace {EscapeModuleName(_module)}");
                writer.WriteLine("{");
                WriteTypeForwardDeclares(writer);
                writer.WriteLine();
                WriteTypeDeclares(writer);
                writer.WriteLine("}");
                writer.WriteLine();

                writer.WriteLine($"namespace {EscapeModuleName(_module)}");
                writer.WriteLine("{");
                WriteTypeMethodsBody(writer, true);
                writer.WriteLine("}");
            }

            using (var writer = new StreamWriter($"{_module.Assembly.Name}.cpp"))
            {
                writer.WriteLine("// Generated by natsu clr compiler.");
                writer.WriteLine($"#include \"{_module.Assembly.Name}.h\"");
                writer.WriteLine();

                writer.WriteLine($"namespace {EscapeModuleName(_module)}");
                writer.WriteLine("{");
                WriteTypeMethodsBody(writer, false);
                writer.WriteLine("}");
            }
        }

        private void SortTypes()
        {
            foreach (var type in _typeDescs.Values)
            {
                foreach (var field in type.TypeDef.Fields)
                    AddTypeRef(type, field.FieldType, false);
                foreach (var method in type.TypeDef.Methods)
                {
                    AddTypeRef(type, method.ReturnType, false);
                    foreach (var param in method.Parameters)
                        AddTypeRef(type, param.Type, false);
                }

                var baseType = GetBaseType(type.TypeDef) as TypeDef;
                if (baseType != null)
                    AddTypeRef(type, baseType, true);
            }

            var visited = new HashSet<TypeDesc>();
            void VisitType(TypeDesc type)
            {
                if (visited.Add(type))
                {
                    foreach (var parent in type.UsedTypes)
                        VisitType(parent);
                    _sortedTypeDescs.Add(type);
                }
            }

            foreach (var type in _typeDescs.Values)
                VisitType(type);
        }

        private ITypeDefOrRef GetBaseType(TypeDef type)
        {
            if (type.FullName == "System.ValueType")
                return null;
            return type.BaseType;
        }

        private void AddTypeRef(TypeDesc declareDesc, TypeSig fieldType, bool force)
        {
            var cntSig = fieldType;
            while (cntSig != null)
            {
                switch (cntSig.ElementType)
                {
                    case ElementType.Void:
                    case ElementType.Var:
                    case ElementType.ByRef:
                        break;
                    case ElementType.Boolean:
                    case ElementType.Char:
                    case ElementType.I1:
                    case ElementType.U1:
                    case ElementType.I2:
                    case ElementType.U2:
                    case ElementType.I4:
                    case ElementType.U4:
                    case ElementType.I8:
                    case ElementType.U8:
                    case ElementType.R4:
                    case ElementType.R8:
                    case ElementType.String:
                        {
                            if (cntSig.TryGetTypeDef() != declareDesc.TypeDef)
                                AddTypeRef(declareDesc, cntSig.TryGetTypeDef(), force);
                        }
                        break;
                    case ElementType.ValueType:
                    case ElementType.Class:
                    case ElementType.Object:
                    case ElementType.SZArray:
                        AddTypeRef(declareDesc, cntSig.TryGetTypeDef(), force);
                        break;
                    default:
                        throw new NotSupportedException();
                }

                cntSig = cntSig.Next;
            }
        }

        private void AddTypeRef(TypeDesc declareDesc, TypeDef typeDef, bool force)
        {
            if (typeDef != null && (force || typeDef.IsValueType))
            {
                var targetDesc = _typeDescs[typeDef];
                if (declareDesc != targetDesc)
                {
                    declareDesc.UsedTypes.Add(targetDesc);
                    targetDesc.UsedByTypes.Add(declareDesc);
                }
            }
        }

        #region Forward Declares
        private void WriteTypeForwardDeclares(StreamWriter writer)
        {
            var types = _typeDescs.Values.Where(x => !x.TypeDef.IsValueType).ToList();
            var index = 0;
            foreach (var type in types)
            {
                WriteTypeForwardDeclare(writer, 0, type);
                if (index++ != types.Count - 1)
                    writer.WriteLine();
            }
        }

        private void WriteTypeForwardDeclare(StreamWriter writer, int ident, TypeDesc type)
        {
            var nss = type.TypeDef.Namespace.String.Split('.', StringSplitOptions.RemoveEmptyEntries)
                .Select(EscapeNamespaceName).ToList();

            writer.Ident(ident);
            foreach (var ns in nss)
                writer.Write($"namespace {ns} {{ ");

            if (type.TypeDef.HasGenericParameters)
            {
                var typeNames = type.TypeDef.GenericParameters.Select(x => "class " + x.Name.String).ToList();
                writer.Ident(ident).Write($"template <{string.Join(", ", typeNames)}> ");
            }

            writer.Ident(ident).Write($"struct {type.Name};");

            foreach (var ns in nss)
                writer.Write(" }");
        }
        #endregion

        #region Declares

        private void WriteTypeDeclares(StreamWriter writer)
        {
            var index = 0;
            foreach (var type in _sortedTypeDescs)
            {
                WriteTypeDeclare(writer, 0, type);
                if (index++ != _sortedTypeDescs.Count - 1)
                    writer.WriteLine();
            }
        }

        private void WriteTypeDeclare(StreamWriter writer, int ident, TypeDesc type)
        {
            var nss = type.TypeDef.Namespace.String.Split('.', StringSplitOptions.RemoveEmptyEntries)
                .Select(EscapeNamespaceName).ToList();

            writer.Ident(ident);
            foreach (var ns in nss)
                writer.Write($"namespace {ns} {{ ");

            writer.WriteLine();
            if (type.TypeDef.IsEnum)
            {
                writer.Ident(ident).WriteLine($"enum class {type.Name} : {GetEnumUnderlyingTypeName(type.TypeDef.GetEnumUnderlyingType().ElementType)}");
                writer.Ident(ident).WriteLine("{");
                foreach (var value in type.TypeDef.Fields)
                {
                    if (value.HasConstant)
                        writer.Ident(ident + 1).WriteLine($"{value.Name} = {value.Constant.Value},");
                }

                writer.Ident(ident).WriteLine("};");
            }
            else
            {
                if (type.TypeDef.HasGenericParameters)
                {
                    var typeNames = type.TypeDef.GenericParameters.Select(x => "class " + x.Name.String).ToList();
                    writer.Ident(ident).WriteLine($"template <{string.Join(", ", typeNames)}> ");
                }

                writer.Ident(ident).Write($"struct {type.Name}");
                var baseType = GetBaseType(type.TypeDef);
                if (baseType != null)
                    writer.WriteLine(" : public " + EscapeTypeName(baseType, isBaseType: true));
                else
                    writer.WriteLine();
                writer.Ident(ident).WriteLine("{");

                foreach (var field in type.TypeDef.Fields)
                {
                    if (field.HasConstant)
                        WriteConstantField(writer, ident + 1, field);
                    else
                        WriteField(writer, ident + 1, field);
                }

                if (type.TypeDef.Fields.Any() && type.TypeDef.Methods.Any())
                    writer.WriteLine();

                foreach (var method in type.TypeDef.Methods)
                {
                    WriteMethodDeclare(writer, ident + 1, method);
                }

                writer.Ident(ident).WriteLine("};");
            }

            foreach (var ns in nss)
                writer.Write("} ");
            writer.WriteLine();
        }

        private string EscapeTypeName(ITypeDefOrRef type, bool isBaseType, bool hasModuleName = false)
        {
            var sb = new StringBuilder();
            if (!isBaseType && !type.IsValueType)
                sb.Append("::natsu::gc_ptr<");

            if (hasModuleName)
                sb.Append("::" + EscapeModuleName(type.DefinitionAssembly) + "::");
            var nss = type.Namespace.Split('.', StringSplitOptions.RemoveEmptyEntries)
                .Select(EscapeNamespaceName).ToList();
            foreach (var ns in nss)
                sb.Append($"{ns}::");
            sb.Append(EscapeTypeName(type.Name));

            if (!isBaseType && !type.IsValueType)
                sb.Append(">");

            return sb.ToString();
        }

        private static string EscapeTypeName(string name)
        {
            return name.Replace('<', '_').Replace('>', '_').Replace('`', '_');
        }

        private void WriteField(StreamWriter writer, int ident, FieldDef value)
        {
            string prefix = string.Empty;
            if (value.IsStatic)
                prefix = "static ";

            writer.Ident(ident).WriteLine($"{prefix}{EscapeTypeName(value.FieldType, value.DeclaringType)} {EscapeIdentifier(value.Name)};");
        }

        private void WriteMethodDeclare(StreamWriter writer, int ident, MethodDef method)
        {
            string prefix = string.Empty;
            if (method.IsStatic)
                prefix = "static ";

            writer.Ident(ident).Write(prefix);
            if (!method.IsInstanceConstructor)
                writer.Write(EscapeTypeName(method.ReturnType) + " ");
            writer.Write(EscapeMethodName(method) + "(");

            var index = 0;
            var parameters = method.IsStatic ? method.Parameters.ToList() : method.Parameters.Skip(1).ToList();
            foreach (var param in parameters)
            {
                writer.Write(EscapeTypeName(param.Type));
                writer.Write(" " + EscapeIdentifier(param.Name));
                if (index++ != parameters.Count - 1)
                    writer.Write(", ");
            }

            writer.WriteLine(");");
        }

        private string EscapeTypeName(TypeSig fieldType, TypeDef declaringType = null)
        {
            var sb = new StringBuilder();
            EscapeTypeName(sb, fieldType, declaringType);
            return sb.ToString();
        }

        private void EscapeTypeName(StringBuilder sb, TypeSig cntSig, TypeDef declaringType = null)
        {
            switch (cntSig.ElementType)
            {
                case ElementType.Void:
                    sb.Append("void");
                    break;
                case ElementType.Boolean:
                case ElementType.Char:
                case ElementType.I1:
                case ElementType.U1:
                case ElementType.I2:
                case ElementType.U2:
                case ElementType.I4:
                case ElementType.U4:
                case ElementType.I8:
                case ElementType.U8:
                case ElementType.R4:
                case ElementType.R8:
                case ElementType.String:
                    {
                        if (cntSig.TryGetTypeDef() == declaringType)
                            sb.Append(GetConstantTypeName(cntSig.ElementType));
                        else
                            sb.Append(EscapeTypeName(cntSig.ToTypeDefOrRef(), isBaseType: false));
                    }
                    break;
                case ElementType.ValueType:
                case ElementType.Class:
                case ElementType.Object:
                    sb.Append(EscapeTypeName(cntSig.ToTypeDefOrRef(), isBaseType: false));
                    break;
                case ElementType.SZArray:
                    sb.Append("::natsu::sz_array<");
                    EscapeTypeName(sb, cntSig.Next, declaringType);
                    sb.Append(">");
                    break;
                case ElementType.Var:
                    sb.Append(cntSig.ToGenericVar().GetName());
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        private void WriteConstantField(StreamWriter writer, int ident, FieldDef value)
        {
            string prefix = string.Empty;
            if (value.IsStatic)
                prefix = "static ";

            writer.Ident(ident).WriteLine($"{prefix}constexpr {GetConstantTypeName(value.ElementType)} {EscapeIdentifier(value.Name)} = {LiteralConstant(value.Constant.Value)}");
        }

        private object LiteralConstant(object value)
        {
            var text = value switch
            {
                char i => ((ushort)i).ToString() + ";",
                byte i => i.ToString() + ";",
                sbyte i => i.ToString() + ";",
                ushort i => i.ToString() + ";",
                short i => i.ToString() + ";",
                uint i => i.ToString() + ";",
                int i => i.ToString() + ";",
                ulong i => i.ToString() + "ULL;",
                long i => "::natsu::to_int64(0x" + Unsafe.As<long, ulong>(ref i).ToString("X") + "); // " + i.ToString(),
                float i => "::natsu::to_float(0x" + Unsafe.As<float, uint>(ref i).ToString("X") + "); // " + i.ToString(),
                double i => "::natsu::to_double(0x" + Unsafe.As<double, ulong>(ref i).ToString("X") + "); // " + i.ToString(),

                _ => throw new NotSupportedException("Unsupported constant")
            };

            return text;
        }
        #endregion

        private void WriteTypeMethodsBody(StreamWriter writer, bool inHeader)
        {
            foreach (var type in _sortedTypeDescs)
            {
                WriteTypeMethodBody(writer, 0, type, inHeader);
            }
        }

        private void WriteTypeMethodBody(StreamWriter writer, int ident, TypeDesc type, bool inHeader)
        {
            foreach (var method in type.TypeDef.Methods)
            {
                if (!method.IsInternalCall)
                {
                    if (inHeader == (type.TypeDef.HasGenericParameters || method.HasGenericParameters))
                    {
                        WriteMethodBody(writer, ident, method);
                        writer.WriteLine();
                    }
                }
            }
        }

        private void WriteMethodBody(StreamWriter writer, int ident, MethodDef method)
        {
            writer.Ident(ident);
            var typeGens = new List<string>();
            var methodGens = new List<string>();

            if (method.DeclaringType.HasGenericParameters)
                typeGens.AddRange(method.DeclaringType.GenericParameters.Select(x => x.Name.String));
            if (method.HasGenericParameters)
                methodGens.AddRange(method.GenericParameters.Select(x => x.Name.String));

            if (typeGens.Any() || methodGens.Any())
                writer.WriteLine($"template <{string.Join(", ", typeGens.Concat(methodGens).Select(x => "class " + x))}>");

            if (!method.IsInstanceConstructor)
                writer.Write(EscapeTypeName(method.ReturnType) + " ");
            writer.Write(EscapeTypeName(method.DeclaringType, true, false));
            if (typeGens.Any())
                writer.Write($"<{string.Join(", ", typeGens)}>");
            writer.Write("::" + EscapeMethodName(method) + "(");

            var index = 0;
            var parameters = method.IsStatic ? method.Parameters.ToList() : method.Parameters.Skip(1).ToList();
            foreach (var param in parameters)
            {
                writer.Write(EscapeTypeName(param.Type));
                writer.Write(" " + EscapeIdentifier(param.Name));
                if (index++ != parameters.Count - 1)
                    writer.Write(", ");
            }

            writer.WriteLine(")");
            writer.Ident(ident).WriteLine("{");
            WriteILBody(writer, ident, method);
            writer.Ident(ident).WriteLine("}");
        }

        private void WriteILBody(StreamWriter writer, int ident, MethodDef method)
        {
            var body = method.Body;
            var stack = new EvaluationStack();
            foreach (var op in body.Instructions)
            {
                WriteInstruction(op, stack, writer, ident, method);
            }
        }

        private void WriteInstruction(Instruction op, EvaluationStack stack, StreamWriter writer, int ident, MethodDef method)
        {
            void ConvertLdarg(int index)
            {
                var param = method.Parameters[index];
                string expr;
                if (!method.IsStatic && index == 0)
                {
                    if (method.DeclaringType.IsValueType)
                        expr = "*this";
                    else
                        expr = "this";
                }
                else
                {
                    expr = EscapeIdentifier(param.Name);
                }

                stack.Push(param.Type, expr);
            }

            void ConvertLdfld(MemberRef member)
            {
                var target = stack.Pop();
                string expr = target.expression + (target.type.IsValueType ? "." : "->") + EscapeIdentifier(member.Name);
                stack.Push(member.FieldSig.Type, expr);
            }

            switch (op.OpCode.Code)
            {
                case Code.Ldarg_0:
                    ConvertLdarg(0);
                    break;
                case Code.Ldarg_1:
                    ConvertLdarg(1);
                    break;
                case Code.Ldarg_2:
                    ConvertLdarg(2);
                    break;
                case Code.Ldarg_3:
                    ConvertLdarg(3);
                    break;
                case Code.Ldarg:
                    ConvertLdarg((int)op.Operand);
                    break;
                case Code.Ldfld:
                    ConvertLdfld((MemberRef)op.Operand);
                    break;
                default:
                    throw new NotSupportedException(op.ToString());
            }
        }

        class EvaluationStack
        {
            private readonly Stack<(TypeSig type, string expression)> _stackValues = new Stack<(TypeSig type, string expression)>();

            public void Push(TypeSig type, string expression)
            {
                _stackValues.Push((type, expression));
            }

            public (TypeSig type, string expression) Pop()
            {
                return _stackValues.Pop();
            }
        }

        private static string GetEnumUnderlyingTypeName(ElementType type)
        {
            switch (type)
            {
                case ElementType.I1:
                    return "int8_t";
                case ElementType.U1:
                    return "uint8_t";
                case ElementType.I2:
                    return "int16_t";
                case ElementType.U2:
                    return "uint16_t";
                case ElementType.I4:
                    return "int32_t";
                case ElementType.U4:
                    return "uint32_t";
                case ElementType.I8:
                    return "int64_t";
                case ElementType.U8:
                    return "uint64_t";
                default:
                    throw new ArgumentException("Invalid enum underlying type");
            }
        }

        private static string GetConstantTypeName(ElementType type)
        {
            switch (type)
            {
                case ElementType.Boolean:
                    return "bool";
                case ElementType.Char:
                    return "char16_t";
                case ElementType.I1:
                    return "int8_t";
                case ElementType.U1:
                    return "uint8_t";
                case ElementType.I2:
                    return "int16_t";
                case ElementType.U2:
                    return "uint16_t";
                case ElementType.I4:
                    return "int32_t";
                case ElementType.U4:
                    return "uint32_t";
                case ElementType.I8:
                    return "int64_t";
                case ElementType.U8:
                    return "uint64_t";
                case ElementType.R4:
                    return "float";
                case ElementType.R8:
                    return "double";
                default:
                    throw new ArgumentException("Invalid constant type");
            }
        }

        private static string EscapeModuleName(ModuleDefMD module)
        {
            return EscapeModuleName(module.Assembly.Name);
        }

        private static string EscapeModuleName(IAssembly assembly)
        {
            return EscapeModuleName(assembly.Name);
        }

        private static string EscapeModuleName(string name)
        {
            return name.Replace('.', '_');
        }

        private static string EscapeMethodName(IMethodDefOrRef method)
        {
            if (method.Name == ".ctor")
                return EscapeTypeName(method.DeclaringType.Name);
            else if (method.Name == ".cctor")
                return "_cctor";
            return EscapeIdentifier(method.Name);
        }

        private static string EscapeNamespaceName(string ns)
        {
            return ns;
        }

        class TypeDesc
        {
            public TypeDef TypeDef { get; }

            public UTF8String Name { get; set; }

            public string QualifiedName { get; set; }

            public Dictionary<UTF8String, TypeDesc> Nested { get; } = new Dictionary<UTF8String, TypeDesc>();

            public HashSet<TypeDesc> UsedTypes { get; } = new HashSet<TypeDesc>();

            public HashSet<TypeDesc> UsedByTypes { get; } = new HashSet<TypeDesc>();

            public TypeDesc(TypeDef typeDef)
            {
                TypeDef = typeDef;
                Name = EscapeName(typeDef.Name);
                QualifiedName = Name;
            }

            public override string ToString()
            {
                return QualifiedName;
            }

            public static string EscapeName(string name)
            {
                return EscapeIdentifier(name);
            }
        }

        private static string EscapeIdentifier(string name)
        {
            return name.Replace('<', '_').Replace('>', '_').Replace('`', '_');
        }
    }

    internal static class Extensions
    {
        public static StreamWriter Ident(this StreamWriter writer, int ident)
        {
            for (int i = 0; i < ident; i++)
                writer.Write("    ");
            return writer;
        }
    }
}
