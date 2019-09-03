using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using dnlib.DotNet;

namespace Natsu.Compiler
{
    static class TypeUtils
    {
        public static StackType GetStackType(TypeSig type)
        {
            switch (type.ElementType)
            {
                case ElementType.Void:
                    return StackType.Void;
                case ElementType.Boolean:
                    return StackType.Int32;
                case ElementType.Char:
                    return StackType.Int32;
                case ElementType.I1:
                    return StackType.Int32;
                case ElementType.U1:
                    return StackType.Int32;
                case ElementType.I2:
                    return StackType.Int32;
                case ElementType.U2:
                    return StackType.Int32;
                case ElementType.I4:
                    return StackType.Int32;
                case ElementType.U4:
                    return StackType.Int32;
                case ElementType.I8:
                    return StackType.Int64;
                case ElementType.U8:
                    return StackType.Int64;
                case ElementType.R4:
                    return StackType.F;
                case ElementType.R8:
                    return StackType.F;
                case ElementType.String:
                    return StackType.O;
                case ElementType.Ptr:
                    return StackType.NativeInt;
                case ElementType.ByRef:
                    return StackType.Ref;
                case ElementType.ValueType:
                    return StackType.ValueType;
                case ElementType.Class:
                    return StackType.O;
                case ElementType.Var:
                    return StackType.Var;
                case ElementType.Array:
                    return StackType.O;
                case ElementType.TypedByRef:
                    return StackType.ValueType;
                case ElementType.I:
                    return StackType.NativeInt;
                case ElementType.U:
                    return StackType.NativeInt;
                case ElementType.R:
                    return StackType.F;
                case ElementType.Object:
                    return StackType.O;
                case ElementType.SZArray:
                    return StackType.O;
                case ElementType.MVar:
                    return StackType.Var;
                case ElementType.GenericInst:
                    {
                        var gen = type.ToGenericInstSig();
                        return GetStackType(gen.GenericType);
                    }
                case ElementType.CModReqd:
                case ElementType.CModOpt:
                case ElementType.Pinned:
                    return GetStackType(type.Next);
                default:
                    throw new NotSupportedException();
            }
        }

        public static TypeSig ThisType(ITypeDefOrRef type)
        {
            if (type.IsValueType)
                return new ByRefSig(type.ToTypeSig());
            return type.ToTypeSig();
        }

        public static string EscapeTypeName(TypeSig fieldType, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null)
        {
            var sb = new StringBuilder();
            EscapeTypeName(sb, fieldType, declaringType, hasGen, genArgs);
            return sb.ToString();
        }

        public static string EscapeVariableTypeName(TypeSig fieldType, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null)
        {
            if (IsValueType(fieldType))
                return EscapeTypeName(fieldType, declaringType, hasGen, genArgs);
            else if (fieldType.IsGenericParameter)
                return $"::natsu::variable_type_t<{EscapeTypeName(fieldType, declaringType, hasGen, genArgs)}>";
            else
                return $"::natsu::gc_obj_ref<{EscapeTypeName(fieldType, declaringType, hasGen, genArgs)}>";
        }

        private static bool IsValueType(TypeSig type)
        {
            if (type == null) return false;
            return type.IsValueType || type.IsByRef || type.IsPointer || (type.IsPinned && IsValueType(type.Next));
        }

        public static void EscapeTypeName(StringBuilder sb, TypeSig cntSig, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null)
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
                case ElementType.I:
                case ElementType.U:
                    {
                        switch (cntSig)
                        {
                            case TypeDefOrRefSig sig:
                                if (declaringType != null && sig.TypeDef == declaringType)
                                    sb.Append(GetConstantTypeName(cntSig.ElementType));
                                else
                                    sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: hasGen-- > 0, genArgs: genArgs));
                                break;
                            default:
                                throw new NotSupportedException();
                        }
                    }
                    break;
                case ElementType.ValueType:
                case ElementType.Class:
                case ElementType.Object:
                    {
                        switch (cntSig)
                        {
                            case TypeDefOrRefSig sig:
                                if (sig.IsValueType && declaringType != null && sig.TypeDef == declaringType)
                                    sb.Append(GetConstantTypeName(cntSig.ElementType));
                                else
                                    sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: hasGen-- > 0, genArgs: genArgs));
                                break;
                            default:
                                throw new NotSupportedException();
                        }
                    }
                    break;
                case ElementType.SZArray:
                    sb.Append("::System_Private_CorLib::System::SZArray_1<");
                    EscapeTypeName(sb, cntSig.Next, declaringType, genArgs: genArgs);
                    sb.Append(">");
                    break;
                case ElementType.Var:
                    sb.Append(cntSig.ToGenericVar().GetName());
                    break;
                case ElementType.MVar:
                    {
                        var mvar = cntSig.ToGenericMVar();
                        if (genArgs != null)
                            EscapeTypeName(sb, genArgs[(int)mvar.Number]);
                        else
                            sb.Append(cntSig.ToGenericMVar().GetName());
                    }
                    break;
                case ElementType.GenericInst:
                    {
                        var sig = cntSig.ToGenericInstSig();
                        sb.Append(EscapeTypeName(sig.GenericType.TypeDefOrRef, hasGen: false));
                        sb.Append("<");
                        for (int i = 0; i < sig.GenericArguments.Count; i++)
                        {
                            EscapeTypeName(sb, sig.GenericArguments[i], null, genArgs: genArgs);
                            if (i != sig.GenericArguments.Count - 1)
                                sb.Append(", ");
                        }
                        sb.Append(">");
                    }
                    break;
                case ElementType.ByRef:
                    sb.Append("::natsu::gc_ref<");
                    sb.Append(EscapeVariableTypeName(cntSig.Next, declaringType, hasGen, genArgs));
                    sb.Append(">");
                    break;
                case ElementType.Ptr:
                    sb.Append("::natsu::gc_ptr<");
                    sb.Append(EscapeVariableTypeName(cntSig.Next, declaringType, hasGen, genArgs));
                    sb.Append(">");
                    break;
                case ElementType.Pinned:
                    EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs);
                    break;
                case ElementType.CModReqd:
                    EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs);
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        public static string EscapeTypeName(ITypeDefOrRef type, bool hasGen = true, bool hasModuleName = true, IList<TypeSig> genArgs = null)
        {
            return EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs);
        }

        public static string EscapeVariableTypeName(ITypeDefOrRef type, bool hasGen = true, bool hasModuleName = true, IList<TypeSig> genArgs = null)
        {
            var sig = type.ToTypeSig();
            if (type.IsValueType || IsValueType(sig))
                return EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs);
            else if (type.IsGenericParam)
                return $"::natsu::variable_type_t<{EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs)}>";
            else
                return $"::natsu::gc_obj_ref<{EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs)}>";
        }

        public static string EscapeTypeName(string name)
        {
            return EscapeIdentifier(name.Split('.').Last());
        }

        private static string EscapeTypeNameImpl(ITypeDefOrRef type, bool hasGen, bool hasModuleName, IList<TypeSig> genArgs = null)
        {
            if (type is TypeSpec typeSpec)
            {
                return EscapeTypeName(typeSpec.TypeSig, null);
            }

            var sb = new StringBuilder();
            if (hasModuleName)
                sb.Append("::" + EscapeModuleName(type.DefinitionAssembly) + "::");
            var nss = type.Namespace.Split('.', StringSplitOptions.RemoveEmptyEntries)
                .Select(EscapeNamespaceName).ToList();
            foreach (var ns in nss)
                sb.Append($"{ns}::");
            sb.Append(EscapeTypeName(type.FullName));
            if (hasGen && type is TypeDef typeDef && typeDef.HasGenericParameters && !type.ContainsGenericParameter)
            {
                sb.Append("<");
                sb.Append(string.Join(", ", typeDef.GenericParameters.Select(x => x.Name)));
                sb.Append(">");
            }

            return sb.ToString();
        }

        public static string EscapeModuleName(ModuleDef module)
        {
            return EscapeModuleName(module.Assembly.Name);
        }

        public static string EscapeModuleName(IAssembly assembly)
        {
            return EscapeModuleName(assembly.Name);
        }

        private static string EscapeModuleName(string name)
        {
            return name.Replace('.', '_');
        }

        public static string EscapeMethodName(IMethod method)
        {
            if (method.MethodSig.HasThis)
                return EscapeIdentifier(method.Name);
            else if (method.Name.EndsWith("op_Explicit"))
                return "_s_" + EscapeIdentifier(method.Name) + "_" + EscapeIdentifier(method.MethodSig.Params[0].FullName) + "_" + EscapeIdentifier(method.MethodSig.RetType.FullName);
            else if (method.Name == ".cctor")
                return "Static";
            else
                return "_s_" + EscapeIdentifier(method.Name);
        }

        public static string EscapeNamespaceName(string ns)
        {
            return ns;
        }

        public static string EscapeIdentifier(string name)
        {
            if (string.IsNullOrEmpty(name))
                throw new ArgumentException("Invalid identifier");

            var sb = new StringBuilder();
            foreach (var c in name)
            {
                if (char.IsLetterOrDigit(c) || c == '_')
                    sb.Append(c);
                else
                    sb.Append('_');
            }

            return sb.ToString();
        }

        public static string GetConstantTypeName(ElementType type)
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
                case ElementType.String:
                    return "::System_Private_CorLib::System::String";
                case ElementType.I:
                    return "intptr_t";
                case ElementType.U:
                    return "uintptr_t";
                default:
                    throw new ArgumentException("Invalid constant type");
            }
        }

        public static string LiteralConstant(object value)
        {
            var text = value switch
            {
                char i => ((ushort)i).ToString(),
                byte i => i.ToString(),
                sbyte i => i.ToString(),
                ushort i => i.ToString(),
                short i => i.ToString(),
                uint i => i.ToString(),
                int i => i.ToString(),
                ulong i => i.ToString() + "ULL",
                long i => "::natsu::to_int64(0x" + Unsafe.As<long, ulong>(ref i).ToString("X") + ") /* " + i.ToString() + "*/",
                float i => "::natsu::to_float(0x" + Unsafe.As<float, uint>(ref i).ToString("X") + ") /* " + i.ToString() + "*/",
                double i => "::natsu::to_double(0x" + Unsafe.As<double, ulong>(ref i).ToString("X") + ") /* " + i.ToString() + "*/",

                _ => throw new NotSupportedException("Unsupported constant")
            };

            return text;
        }

        public static IList<TypeSig> InstantiateGenericTypes(IList<TypeSig> types, IList<TypeSig> genericArguments)
        {
            var newTypes = new List<TypeSig>(types.Count);
            for (int i = 0; i < types.Count; i++)
            {
                var oldType = types[i];
                if (oldType.IsGenericParameter)
                {
                    var gen = genericArguments.FirstOrDefault(x => x.GetName() == oldType.GetName());
                    if (gen != null)
                    {
                        newTypes.Add(gen);
                        continue;
                    }
                }

                newTypes.Add(oldType);
            }

            return newTypes;
        }
    }
}
