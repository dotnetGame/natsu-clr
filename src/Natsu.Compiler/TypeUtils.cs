using System;
using System.Collections.Generic;
using System.IO;
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
            StackTypeCode code;
            switch (type.ElementType)
            {
                case ElementType.Void:
                    code = StackTypeCode.Void;
                    break;
                case ElementType.Boolean:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.Char:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.I1:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.U1:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.I2:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.U2:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.I4:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.U4:
                    code = StackTypeCode.Int32;
                    break;
                case ElementType.I8:
                    code = StackTypeCode.Int64;
                    break;
                case ElementType.U8:
                    code = StackTypeCode.Int64;
                    break;
                case ElementType.R4:
                    code = StackTypeCode.F;
                    break;
                case ElementType.R8:
                    code = StackTypeCode.F;
                    break;
                case ElementType.String:
                    code = StackTypeCode.O;
                    break;
                case ElementType.Ptr:
                    code = StackTypeCode.NativeInt;
                    break;
                case ElementType.ByRef:
                    code = StackTypeCode.Ref;
                    break;
                case ElementType.ValueType:
                    code = StackTypeCode.Runtime;
                    break;
                case ElementType.Class:
                    code = StackTypeCode.O;
                    break;
                case ElementType.Var:
                    code = StackTypeCode.Runtime;
                    break;
                case ElementType.Array:
                    code = StackTypeCode.O;
                    break;
                case ElementType.TypedByRef:
                    code = StackTypeCode.Runtime;
                    break;
                case ElementType.I:
                    code = StackTypeCode.NativeInt;
                    break;
                case ElementType.U:
                    code = StackTypeCode.NativeInt;
                    break;
                case ElementType.R:
                    code = StackTypeCode.F;
                    break;
                case ElementType.Object:
                    code = StackTypeCode.O;
                    break;
                case ElementType.SZArray:
                    code = StackTypeCode.O;
                    break;
                case ElementType.MVar:
                    code = StackTypeCode.Runtime;
                    break;
                case ElementType.GenericInst:
                    {
                        var gen = type.ToGenericInstSig();
                        code = GetStackType(gen.GenericType).Code;
                        break;
                    }
                case ElementType.CModReqd:
                case ElementType.CModOpt:
                case ElementType.Pinned:
                    return GetStackType(type.Next);
                default:
                    throw new NotSupportedException();
            }

            return new StackType { Code = code, Name = EscapeVariableTypeName(type) };
        }

        public static TypeSig ThisType(ITypeDefOrRef type)
        {
            var typeDef = type.ResolveTypeDef();
            if (typeDef?.IsValueType ?? false || type.IsValueType)
                return new ByRefSig(type.ToTypeSig());
            return type.ToTypeSig();
        }

        public static string EscapeTypeName(TypeSig fieldType, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null, bool cppBasicType = false)
        {
            var sb = new StringBuilder();
            EscapeTypeName(sb, fieldType, declaringType, hasGen, genArgs, cppBasicType);
            return sb.ToString();
        }

        public static string EscapeVariableTypeName(TypeSig fieldType, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null)
        {
            if (fieldType.ElementType == ElementType.CModReqd)
            {
                var modifier = ((ModifierSig)fieldType).Modifier;
                var modName = modifier.FullName;
                if (modName == "System.Runtime.CompilerServices.IsVolatile")
                {
                    return $"::natsu::clr_volatile<{EscapeVariableTypeName(fieldType.Next, declaringType, hasGen, genArgs)}>";
                }
            }

            if (IsValueType(fieldType))
                return EscapeTypeName(fieldType, declaringType, hasGen, genArgs, cppBasicType: true);
            else if (fieldType.IsGenericParameter)
                return $"::natsu::variable_type_t<{EscapeTypeName(fieldType, declaringType, hasGen, genArgs, cppBasicType: true)}>";
            else
                return $"::natsu::gc_obj_ref<{EscapeTypeName(fieldType, declaringType, hasGen, genArgs, cppBasicType: true)}>";
        }

        private static bool IsValueType(TypeSig type)
        {
            if (type == null) return false;
            return type.IsValueType || type.IsByRef || type.IsPointer || (type.IsPinned && IsValueType(type.Next)) || (type.IsModifier && IsValueType(type.Next));
        }

        public static void EscapeTypeName(StringBuilder sb, TypeSig cntSig, TypeDef declaringType = null, int hasGen = 0, IList<TypeSig> genArgs = null, bool cppBasicType = false)
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
                                    sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: hasGen-- > 0, genArgs: genArgs, cppBasicType: cppBasicType));
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
                                if (sig.IsPrimitive && declaringType != null && sig.TypeDef == declaringType)
                                    sb.Append(GetConstantTypeName(cntSig.ElementType));
                                else
                                    sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: hasGen-- > 0, genArgs: genArgs, cppBasicType: cppBasicType));
                                break;
                            default:
                                throw new NotSupportedException();
                        }
                    }
                    break;
                case ElementType.SZArray:
                    sb.Append("::System_Private_CoreLib::System::SZArray_1<");
                    EscapeTypeName(sb, cntSig.Next, declaringType, genArgs: genArgs, cppBasicType: true);
                    sb.Append(">");
                    break;
                case ElementType.Var:
                    {
                        var var = cntSig.ToGenericVar();
                        if (genArgs != null)
                        {
                            var sig = genArgs.OfType<GenericSig>().FirstOrDefault(x => x.Number == var.Number);
                            if (sig != null)
                                EscapeTypeName(sb, sig, cppBasicType: true);
                            else
                                EscapeTypeName(sb, genArgs[(int)var.Number], cppBasicType: true);
                        }
                        else
                        {
                            sb.Append(var.GetName());
                        }
                    }
                    break;
                case ElementType.MVar:
                    {
                        var mvar = cntSig.ToGenericMVar();
                        if (genArgs != null)
                            EscapeTypeName(sb, genArgs[(int)mvar.Number], cppBasicType: true);
                        else
                            sb.Append(mvar.GetName());
                    }
                    break;
                case ElementType.GenericInst:
                    {
                        var sig = cntSig.ToGenericInstSig();
                        sb.Append(EscapeTypeName(sig.GenericType.TypeDefOrRef, hasGen: false, cppBasicType: cppBasicType));
                        sb.Append("<");
                        for (int i = 0; i < sig.GenericArguments.Count; i++)
                        {
                            EscapeTypeName(sb, sig.GenericArguments[i], null, genArgs: genArgs, cppBasicType: true);
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
                    EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs, cppBasicType: cppBasicType);
                    break;
                case ElementType.CModReqd:
                    {
                        var modifier = ((ModifierSig)cntSig).Modifier;
                        var modName = modifier.FullName;
                        if (modName == "System.Runtime.InteropServices.InAttribute")
                        {
                            EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs, cppBasicType: cppBasicType);
                        }
                        else if (modName == "System.Runtime.CompilerServices.IsVolatile")
                        {
                            sb.Append("::natsu::clr_volatile<");
                            EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs, cppBasicType: cppBasicType);
                            sb.Append(">");
                        }
                        else
                        {
                            EscapeTypeName(sb, cntSig.Next, declaringType, hasGen, genArgs, cppBasicType: cppBasicType);
                        }
                        break;
                    }
                default:
                    throw new NotSupportedException();
            }
        }

        public static string EscapeTypeName(ITypeDefOrRef type, bool hasGen = true, bool hasModuleName = true, IList<TypeSig> genArgs = null, bool cppBasicType = false)
        {
            return EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs, cppBasicType);
        }

        public static string EscapeVariableTypeName(ITypeDefOrRef type, bool hasGen = true, bool hasModuleName = true, IList<TypeSig> genArgs = null)
        {
            var sig = type.ToTypeSig();
            if (type.IsValueType || IsValueType(sig))
                return EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs);
            else if (type.ToTypeSig().IsGenericParameter)
                return $"::natsu::variable_type_t<{EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs)}>";
            else
                return $"::natsu::gc_obj_ref<{EscapeTypeNameImpl(type, hasGen, hasModuleName, genArgs)}>";
        }

        public static string EscapeTypeName(string name)
        {
            return EscapeIdentifier(name.Split('.').Last());
        }

        public static bool IsCppBasicType(ITypeDefOrRef type)
        {
            switch (type.ToTypeSig().ElementType)
            {
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
                    return true;
                    //case ElementType.I:
                    //case ElementType.U:
            }

            return false;
        }

        private static string EscapeTypeNameImpl(ITypeDefOrRef type, bool hasGen, bool hasModuleName, IList<TypeSig> genArgs = null, bool cppBasicType = false)
        {
            if (type is TypeSpec typeSpec)
            {
                return EscapeTypeName(typeSpec.TypeSig, null, cppBasicType: cppBasicType);
            }

            if (cppBasicType && type.IsPrimitive)
            {
                switch (type.ToTypeSig().ElementType)
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
                        //case ElementType.I:
                        //    return "intptr_t";
                        //case ElementType.U:
                        //    return "uintptr_t";
                }
            }

            var sb = new StringBuilder();
            if (hasModuleName)
            {
                var moduleName = EscapeModuleName(type.DefinitionAssembly);
                if (moduleName == "mscorlib")
                    moduleName = "System_Private_CoreLib";
                sb.Append("::" + moduleName + "::");
            }

            var nss = TypeUtils.GetNamespace(type).Split('.', StringSplitOptions.RemoveEmptyEntries)
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

        public static string EscapeMethodParamsType(IMethod method)
        {
            var sb = new StringBuilder();
            for (int i = 0; i < method.MethodSig.Params.Count; i++)
            {
                sb.Append(EscapeVariableTypeName(method.MethodSig.Params[i]));
                if (i != method.MethodSig.Params.Count - 1)
                    sb.Append(", ");
            }

            return sb.ToString();
        }

        public static string EscapeMethodName(IMethod method, bool hasParamType = true, bool hasExplicit = false)
        {
            if (method.Name == ".ctor")
                return "_ctor";
            if (method.MethodSig.HasThis)
            {
                var sb = new StringBuilder();
                if (hasExplicit)
                    sb.Append(EscapeIdentifier(method.Name.String));
                else
                    sb.Append(EscapeIdentifier(method.Name.String.Split('.').Last()));

                if (hasParamType)
                {
                    sb.Append("_");

                    for (int i = 0; i < method.MethodSig.Params.Count; i++)
                    {
                        EscapeMethodTypeName(sb, method.MethodSig.Params[i]);
                        if (i != method.MethodSig.Params.Count - 1)
                            sb.Append("_");
                    }
                }

                var name = EscapeIdentifier(sb.ToString());

                return name;
            }
            else if (method.Name.EndsWith("op_Explicit"))
                return "_s_" + EscapeIdentifier(method.Name) + "_" + EscapeIdentifier(method.MethodSig.Params[0].FullName) + "_" + EscapeIdentifier(method.MethodSig.RetType.FullName);
            else if (method.Name == ".cctor")
                return "Static";
            else
                return "_s_" + EscapeIdentifier(method.Name);
        }

        public static void EscapeMethodTypeName(StringBuilder sb, TypeSig cntSig)
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
                                sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: false, hasModuleName: false));
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
                                sb.Append(EscapeTypeName(sig.TypeDefOrRef, hasGen: false, hasModuleName: false));
                                break;
                            default:
                                throw new NotSupportedException();
                        }
                    }
                    break;
                case ElementType.SZArray:
                    sb.Append("System_SZArray_1_");
                    EscapeTypeName(sb, cntSig.Next);
                    break;
                case ElementType.Var:
                    {
                        var var = cntSig.ToGenericVar();
                        sb.Append(var.GetName());
                    }
                    break;
                case ElementType.MVar:
                    {
                        var mvar = cntSig.ToGenericMVar();
                        sb.Append(mvar.GetName());
                    }
                    break;
                case ElementType.GenericInst:
                    {
                        var sig = cntSig.ToGenericInstSig();
                        sb.Append(EscapeTypeName(sig.GenericType.TypeDefOrRef, hasGen: false, hasModuleName: false));
                        sb.Append("_");
                        for (int i = 0; i < sig.GenericArguments.Count; i++)
                        {
                            EscapeMethodTypeName(sb, sig.GenericArguments[i]);
                            if (i != sig.GenericArguments.Count - 1)
                                sb.Append("_ ");
                        }
                    }
                    break;
                case ElementType.ByRef:
                    sb.Append("ref_");
                    EscapeMethodTypeName(sb, cntSig.Next);
                    break;
                case ElementType.Ptr:
                    sb.Append("ptr_");
                    EscapeMethodTypeName(sb, cntSig.Next);
                    break;
                case ElementType.Pinned:
                    EscapeMethodTypeName(sb, cntSig.Next);
                    break;
                case ElementType.CModReqd:
                    EscapeMethodTypeName(sb, cntSig.Next);
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        public static string GetNamespace(ITypeDefOrRef type)
        {
            if (type is TypeDef typeDef)
            {
                if (typeDef.IsNested)
                    return typeDef.DeclaringType.Namespace;
            }
            else if (type is TypeRef typeRef)
            {
                if (typeRef.IsNested)
                    return typeRef.DeclaringType.Namespace;
            }

            return type.Namespace;
        }

        public static string GetTypeSize(ElementType elementType)
        {
            switch (elementType)
            {
                case ElementType.Boolean:
                case ElementType.I1:
                case ElementType.U1:
                    return "1";
                case ElementType.Char:
                case ElementType.I2:
                case ElementType.U2:
                    return "2";
                case ElementType.I4:
                case ElementType.U4:
                case ElementType.R4:
                    return "4";
                case ElementType.I8:
                case ElementType.U8:
                case ElementType.R8:
                    return "8";
                case ElementType.String:
                case ElementType.Ptr:
                case ElementType.ByRef:
                case ElementType.Class:
                case ElementType.I:
                case ElementType.U:
                case ElementType.Object:
                case ElementType.SZArray:
                case ElementType.FnPtr:
                    return "sizeof(intptr_t)";
                default:
                    throw new InvalidProgramException();
            }
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
            if (char.IsDigit(name[0]))
                sb.Append('_');
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
                    return "::System_Private_CoreLib::System::String";
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
                float i => EscapeFloat(i),
                double i => EscapeFloat(i),

                _ => throw new NotSupportedException("Unsupported constant")
            };

            return text;
        }

        private static string EscapeFloat(float v)
        {
            if (float.IsNaN(v))
                return "std::numeric_limits<float>::quiet_NaN()";
            else if (float.IsNegativeInfinity(v))
                return "-std::numeric_limits<float>::infinity()";
            else if (float.IsPositiveInfinity(v))
                return "std::numeric_limits<float>::infinity()";
            else if (v == float.MinValue)
                return "std::numeric_limits<float>::lowest()";
            else if (v == float.MaxValue)
                return "std::numeric_limits<float>::max()";
            else if (v == float.Epsilon)
                return "std::numeric_limits<float>::denorm_min()";
            else
            {
                var str = v.ToString("E");
                if (str.Contains('.') || str.Contains('E'))
                    return str + "F";
                return str + ".F";
            }
        }

        private static string EscapeFloat(double v)
        {
            if (double.IsNaN(v))
                return "std::numeric_limits<double>::quiet_NaN()";
            else if (double.IsNegativeInfinity(v))
                return "-std::numeric_limits<double>::infinity()";
            else if (double.IsPositiveInfinity(v))
                return "std::numeric_limits<double>::infinity()";
            else if (v == double.MinValue)
                return "std::numeric_limits<double>::lowest()";
            else if (v == double.MaxValue)
                return "std::numeric_limits<double>::max()";
            else if (v == double.Epsilon)
                return "std::numeric_limits<double>::denorm_min()";
            else
            {
                var str = v.ToString("E");
                if (str.Contains('.') || str.Contains('E'))
                    return str;
                return str + ".";
            }
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

        public static string EscapeStackTypeName(StackType stackType)
        {
            switch (stackType.Code)
            {
                case StackTypeCode.Int32:
                    return "::natsu::stack::int32";
                case StackTypeCode.Int64:
                    return "::natsu::stack::int64";
                case StackTypeCode.NativeInt:
                    return "::natsu::stack::native_int";
                case StackTypeCode.F:
                    return "::natsu::stack::F";
                case StackTypeCode.O:
                    return "::natsu::stack::O";
                case StackTypeCode.Ref:
                    return "::natsu::stack::Ref";
                default:
                    if (string.IsNullOrEmpty(stackType.Name))
                        throw new NotSupportedException(stackType.ToString());
                    return stackType.Name;
            }
        }

        public static string EscapeCSharpTypeName(string name)
        {
            return EscapeCSharpTypeNameCore(new StringReader(name));
        }

        private static string EscapeCSharpTypeNameCore(TextReader name)
        {
            var sb = new StringBuilder();
            var genSb = new StringBuilder();
            int count = 0;
            while (name.Peek() != -1)
            {
                var c = (char)name.Peek();
                if (c == '.')
                {
                    name.Read();
                    sb.Append("::");
                }
                else if (c == '<')
                {
                    name.Read();
                    count = 1;
                    sb.Append("_");
                    var nest = EscapeCSharpTypeNameCore(name);
                    genSb.Append('<');
                    genSb.Append(nest);
                }
                else if (c == ',')
                {
                    name.Read();
                    if (count != 0)
                    {
                        count++;
                        var nest = EscapeCSharpTypeNameCore(name);
                        genSb.Append(", ");
                        genSb.Append(nest);
                    }
                    else
                    {
                        break;
                    }
                }
                else if (c == '>')
                {
                    if (count != 0)
                    {
                        name.Read();
                        sb.Append(count);
                        sb.Append(genSb);
                        sb.Append('>');
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    name.Read();
                    sb.Append(c);
                }
            }

            return sb.ToString();
        }

        public static bool IsSameType(ITypeDefOrRef type1, ITypeDefOrRef type2)
        {
            return type1 == type2 || type1 == type2.Scope;
        }
    }
}
