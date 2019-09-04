using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace Natsu.Compiler
{
    class ILImporter
    {
        private BasicBlock _headBlock;
        private readonly StreamWriter _writer;
        private readonly int _ident;
        private readonly MethodDef _method;

        public ILImporter(MethodDef method, StreamWriter writer, int ident)
        {
            _method = method;
            _writer = writer;
            _ident = ident;
        }

        public void ImportBlocks(IList<Instruction> instructions)
        {
            int id = 0;
            Instruction NextInst(Instruction inst)
            {
                var index = instructions.IndexOf(inst);
                if (index < instructions.Count - 1)
                    return instructions[index + 1];
                return null;
            }

            BasicBlock ImportBlock(Instruction inst)
            {
                if (inst == null) return null;

                var block = new BasicBlock { Id = id++ };
                bool conti = true;

                void AddNext(Instruction next)
                {
                    if (block.Instructions[0] != next)
                        block.Next.Add(ImportBlock(next));
                }

                while (conti)
                {
                    switch (inst.OpCode.Code)
                    {
                        case Code.Br:
                        case Code.Br_S:
                            block.Instructions.Add(inst);
                            AddNext((Instruction)inst.Operand);
                            conti = false;
                            break;
                        case Code.Brfalse:
                        case Code.Brfalse_S:
                        case Code.Brtrue:
                        case Code.Brtrue_S:
                        case Code.Blt_S:
                        case Code.Blt:
                        case Code.Blt_Un_S:
                        case Code.Blt_Un:
                        case Code.Ble_S:
                        case Code.Ble:
                        case Code.Ble_Un:
                        case Code.Ble_Un_S:
                        case Code.Beq_S:
                        case Code.Beq:
                        case Code.Bge_S:
                        case Code.Bge:
                        case Code.Bge_Un_S:
                        case Code.Bge_Un:
                        case Code.Bgt_S:
                        case Code.Bgt:
                        case Code.Bgt_Un_S:
                        case Code.Bgt_Un:
                        case Code.Bne_Un_S:
                        case Code.Bne_Un:
                            block.Instructions.Add(inst);
                            AddNext((Instruction)inst.Operand);
                            AddNext(NextInst(inst));
                            conti = false;
                            break;
                        case Code.Ret:
                        case Code.Throw:
                            block.Instructions.Add(inst);
                            conti = false;
                            break;
                        case Code.Switch:
                            block.Instructions.Add(inst);
                            foreach (var c in (IEnumerable<Instruction>)inst.Operand)
                                AddNext(c);
                            AddNext(NextInst(inst));
                            conti = false;
                            break;
                        default:
                            if (inst.OpCode.Code != Code.Box && inst.OpCode.Code.ToString().StartsWith('B'))
                                throw new NotImplementedException();

                            block.Instructions.Add(inst);
                            inst = NextInst(inst);
                            break;
                    }
                }

                return block;
            }

            if (instructions.Count != 0)
                _headBlock = ImportBlock(instructions[0]);
            else
                _headBlock = new BasicBlock { Id = 0 };
        }

        internal void Gencode()
        {
            var stack = new EvaluationStack(_writer, _ident);
            VisitBlock(_ident, _headBlock, stack);
        }

        private void VisitBlock(int ident, BasicBlock block, EvaluationStack evaluationStack)
        {
            _writer.WriteLine(ILUtils.GetLabel(_method, block.Id) + ":");

            foreach (var op in block.Instructions)
            {
                WriteInstruction(op, evaluationStack, ident, block);
                _writer.Flush();
            }

            foreach (var next in block.Next)
            {
                _writer.Ident(ident).WriteLine("{");
                VisitBlock(ident + 1, next, evaluationStack.Clone(1));
                _writer.Ident(ident).WriteLine("}");
            }
        }

        private void WriteInstruction(Instruction op, EvaluationStack stack, int ident, BasicBlock block)
        {
            var emitter = new OpEmitter { Method = _method, Op = op, Stack = stack, Ident = ident, Block = block, Writer = _writer };
            bool isSpecial = true;

            if (op.IsLdarg())
                emitter.Ldarg();
            else if (op.IsStarg())
                emitter.Starg();
            else if (op.IsLdcI4())
                emitter.LdcI4();
            else if (op.IsLdloc())
                emitter.Ldloc();
            else if (op.IsStloc())
                emitter.Stloc();
            else if (op.IsBr())
                emitter.Br();
            else if (op.IsBrtrue())
                emitter.Brtrue();
            else if (op.IsBrfalse())
                emitter.Brfalse();
            else
                isSpecial = false;

            if (!isSpecial)
            {
                switch (op.OpCode.Code)
                {
                    case Code.Call:
                        emitter.Call();
                        break;
                    case Code.Callvirt:
                        emitter.Callvirt();
                        break;
                    case Code.Nop:
                        emitter.Nop();
                        break;
                    case Code.Ret:
                        emitter.Ret();
                        break;
                    case Code.Throw:
                        emitter.Throw();
                        break;
                    case Code.Neg:
                        emitter.Neg();
                        break;
                    case Code.Not:
                        emitter.Not();
                        break;
                    case Code.Add:
                        emitter.Add();
                        break;
                    case Code.Sub:
                        emitter.Sub();
                        break;
                    case Code.Mul:
                        emitter.Mul();
                        break;
                    case Code.Div:
                        emitter.Div();
                        break;
                    case Code.Div_Un:
                        emitter.Div_Un();
                        break;
                    case Code.Rem:
                        emitter.Rem();
                        break;
                    case Code.Rem_Un:
                        emitter.Rem_Un();
                        break;
                    case Code.And:
                        emitter.And();
                        break;
                    case Code.Or:
                        emitter.Or();
                        break;
                    case Code.Xor:
                        emitter.Xor();
                        break;
                    case Code.Shl:
                        emitter.Shl();
                        break;
                    case Code.Shr:
                        emitter.Shr();
                        break;
                    case Code.Shr_Un:
                        emitter.Shr_Un();
                        break;
                    case Code.Clt:
                        emitter.Clt();
                        break;
                    case Code.Clt_Un:
                        emitter.Clt_Un();
                        break;
                    case Code.Ceq:
                        emitter.Ceq();
                        break;
                    case Code.Cgt:
                        emitter.Cgt();
                        break;
                    case Code.Cgt_Un:
                        emitter.Cgt_Un();
                        break;
                    case Code.Ldc_I8:
                        emitter.Ldc_I8();
                        break;
                    case Code.Ldc_R4:
                        emitter.Ldc_R4();
                        break;
                    case Code.Ldc_R8:
                        emitter.Ldc_R8();
                        break;
                    case Code.Ldstr:
                        emitter.Ldstr();
                        break;
                    case Code.Ldsfld:
                        emitter.Ldsfld();
                        break;
                    case Code.Ldfld:
                        emitter.Ldfld();
                        break;
                    case Code.Stsfld:
                        emitter.Stsfld();
                        break;
                    case Code.Stfld:
                        emitter.Stfld();
                        break;
                    case Code.Ldarga_S:
                    case Code.Ldarga:
                        emitter.Ldarga();
                        break;
                    case Code.Ldloca_S:
                    case Code.Ldloca:
                        emitter.Ldloca();
                        break;
                    case Code.Ldflda:
                        emitter.Ldflda();
                        break;
                    case Code.Ldsflda:
                        emitter.Ldsflda();
                        break;
                    case Code.Ldind_I1:
                        emitter.Ldind_I1();
                        break;
                    case Code.Ldind_I2:
                        emitter.Ldind_I2();
                        break;
                    case Code.Ldind_I4:
                        emitter.Ldind_I4();
                        break;
                    case Code.Ldind_I8:
                        emitter.Ldind_I8();
                        break;
                    case Code.Ldind_R4:
                        emitter.Ldind_R4();
                        break;
                    case Code.Ldind_R8:
                        emitter.Ldind_R8();
                        break;
                    case Code.Ldind_I:
                        emitter.Ldind_I();
                        break;
                    case Code.Ldind_Ref:
                        emitter.Ldind_Ref();
                        break;
                    case Code.Ldind_U1:
                        emitter.Ldind_U1();
                        break;
                    case Code.Ldind_U2:
                        emitter.Ldind_U2();
                        break;
                    case Code.Ldind_U4:
                        emitter.Ldind_U4();
                        break;
                    case Code.Stind_I1:
                        emitter.Stind_I1();
                        break;
                    case Code.Stind_I2:
                        emitter.Stind_I2();
                        break;
                    case Code.Stind_I4:
                        emitter.Stind_I4();
                        break;
                    case Code.Stind_I8:
                        emitter.Stind_I8();
                        break;
                    case Code.Stind_R4:
                        emitter.Stind_R4();
                        break;
                    case Code.Stind_R8:
                        emitter.Stind_R8();
                        break;
                    case Code.Stind_Ref:
                        emitter.Stind_Ref();
                        break;
                    case Code.Ldelem_I1:
                        emitter.Ldelem_I1();
                        break;
                    case Code.Ldelem_I2:
                        emitter.Ldelem_I2();
                        break;
                    case Code.Ldelem_I4:
                        emitter.Ldelem_I4();
                        break;
                    case Code.Ldelem_I8:
                        emitter.Ldelem_I8();
                        break;
                    case Code.Ldelem_R4:
                        emitter.Ldelem_R4();
                        break;
                    case Code.Ldelem_R8:
                        emitter.Ldelem_R8();
                        break;
                    case Code.Ldelem_I:
                        emitter.Ldelem_I();
                        break;
                    case Code.Ldelem_Ref:
                        emitter.Ldelem_Ref();
                        break;
                    case Code.Ldelem_U1:
                        emitter.Ldelem_U1();
                        break;
                    case Code.Ldelem_U2:
                        emitter.Ldelem_U2();
                        break;
                    case Code.Ldelem_U4:
                        emitter.Ldelem_U4();
                        break;
                    case Code.Stelem_I1:
                        emitter.Stelem_I1();
                        break;
                    case Code.Stelem_I2:
                        emitter.Stelem_I2();
                        break;
                    case Code.Stelem_I4:
                        emitter.Stelem_I4();
                        break;
                    case Code.Stelem_I8:
                        emitter.Stelem_I8();
                        break;
                    case Code.Stelem_R4:
                        emitter.Stelem_R4();
                        break;
                    case Code.Stelem_R8:
                        emitter.Stelem_R8();
                        break;
                    case Code.Stelem_Ref:
                        emitter.Stelem_Ref();
                        break;
                    case Code.Ldelema:
                        emitter.Ldelema();
                        break;
                    case Code.Ldnull:
                        emitter.Ldnull();
                        break;
                    case Code.Newarr:
                        emitter.Newarr();
                        break;
                    case Code.Initobj:
                        emitter.Initobj();
                        break;
                    case Code.Box:
                        emitter.Box();
                        break;
                    case Code.Newobj:
                        emitter.Newobj();
                        break;
                    case Code.Ldobj:
                        emitter.Ldobj();
                        break;
                    case Code.Stobj:
                        emitter.Stobj();
                        break;
                    case Code.Ldtoken:
                        emitter.Ldtoken();
                        break;
                    case Code.Isinst:
                        emitter.Isinst();
                        break;
                    case Code.Unbox_Any:
                        emitter.Unbox_Any();
                        break;
                    case Code.Unbox:
                        emitter.Unbox();
                        break;
                    case Code.Ldlen:
                        emitter.Ldlen();
                        break;
                    case Code.Conv_I1:
                        emitter.Conv_I1();
                        break;
                    case Code.Conv_I2:
                        emitter.Conv_I2();
                        break;
                    case Code.Conv_I4:
                        emitter.Conv_I4();
                        break;
                    case Code.Conv_I8:
                        emitter.Conv_I8();
                        break;
                    case Code.Conv_I:
                        emitter.Conv_I();
                        break;
                    case Code.Conv_R4:
                        emitter.Conv_R4();
                        break;
                    case Code.Conv_R8:
                        emitter.Conv_R8();
                        break;
                    case Code.Conv_U1:
                        emitter.Conv_U1();
                        break;
                    case Code.Conv_U2:
                        emitter.Conv_U2();
                        break;
                    case Code.Conv_U4:
                        emitter.Conv_U4();
                        break;
                    case Code.Conv_U8:
                        emitter.Conv_U8();
                        break;
                    case Code.Conv_U:
                        emitter.Conv_U();
                        break;
                    case Code.Conv_Ovf_I1:
                        emitter.Conv_Ovf_I1();
                        break;
                    case Code.Conv_Ovf_I2:
                        emitter.Conv_Ovf_I2();
                        break;
                    case Code.Conv_Ovf_I4:
                        emitter.Conv_Ovf_I4();
                        break;
                    case Code.Conv_Ovf_I8:
                        emitter.Conv_Ovf_I8();
                        break;
                    case Code.Conv_Ovf_I:
                        emitter.Conv_Ovf_I();
                        break;
                    case Code.Conv_Ovf_U1:
                        emitter.Conv_Ovf_U1();
                        break;
                    case Code.Conv_Ovf_U2:
                        emitter.Conv_Ovf_U2();
                        break;
                    case Code.Conv_Ovf_U4:
                        emitter.Conv_Ovf_U4();
                        break;
                    case Code.Conv_Ovf_U8:
                        emitter.Conv_Ovf_U8();
                        break;
                    case Code.Conv_Ovf_U:
                        emitter.Conv_Ovf_U();
                        break;
                    case Code.Conv_Ovf_I1_Un:
                        emitter.Conv_Ovf_I1_Un();
                        break;
                    case Code.Conv_Ovf_I2_Un:
                        emitter.Conv_Ovf_I2_Un();
                        break;
                    case Code.Conv_Ovf_I4_Un:
                        emitter.Conv_Ovf_I4_Un();
                        break;
                    case Code.Conv_Ovf_I8_Un:
                        emitter.Conv_Ovf_I8_Un();
                        break;
                    case Code.Conv_Ovf_I_Un:
                        emitter.Conv_Ovf_I_Un();
                        break;
                    case Code.Conv_Ovf_U1_Un:
                        emitter.Conv_Ovf_U1_Un();
                        break;
                    case Code.Conv_Ovf_U2_Un:
                        emitter.Conv_Ovf_U2_Un();
                        break;
                    case Code.Conv_Ovf_U4_Un:
                        emitter.Conv_Ovf_U4_Un();
                        break;
                    case Code.Conv_Ovf_U8_Un:
                        emitter.Conv_Ovf_U8_Un();
                        break;
                    case Code.Conv_Ovf_U_Un:
                        emitter.Conv_Ovf_U_Un();
                        break;
                    case Code.Blt_S:
                    case Code.Blt:
                        emitter.Blt();
                        break;
                    case Code.Blt_Un_S:
                    case Code.Blt_Un:
                        emitter.Blt_Un();
                        break;
                    case Code.Ble_S:
                    case Code.Ble:
                        emitter.Ble();
                        break;
                    case Code.Ble_Un_S:
                    case Code.Ble_Un:
                        emitter.Ble_Un();
                        break;
                    case Code.Beq_S:
                    case Code.Beq:
                        emitter.Beq();
                        break;
                    case Code.Bge_S:
                    case Code.Bge:
                        emitter.Bge();
                        break;
                    case Code.Bge_Un_S:
                    case Code.Bge_Un:
                        emitter.Bge_Un();
                        break;
                    case Code.Bgt_S:
                    case Code.Bgt:
                        emitter.Bgt();
                        break;
                    case Code.Bgt_Un_S:
                    case Code.Bgt_Un:
                        emitter.Bgt_Un();
                        break;
                    case Code.Bne_Un_S:
                    case Code.Bne_Un:
                        emitter.Bne_Un();
                        break;
                    case Code.Switch:
                        emitter.Switch();
                        break;
                    case Code.Dup:
                        emitter.Dup();
                        break;
                    case Code.Pop:
                        stack.Pop();
                        break;
                    case Code.Constrained:
                        stack.Constrained = (ITypeDefOrRef)op.Operand;
                        break;
                    default:
                        throw new NotSupportedException(op.OpCode.Code.ToString());
                }
            }
        }
    }

    class OpEmitter
    {
        public Instruction Op { get; set; }
        public EvaluationStack Stack { get; set; }
        public int Ident { get; set; }
        public BasicBlock Block { get; set; }
        public MethodDef Method { get; set; }
        public StreamWriter Writer { get; set; }

        // Unary

        public void Neg() => Unary("neg");
        public void Not() => Unary("not");

        // Binary

        public void Add() => Binary("add");
        public void Sub() => Binary("sub");
        public void Mul() => Binary("mul");
        public void Div() => Binary("div");
        public void Div_Un() => Binary("div_un");
        public void Rem() => Binary("rem");
        public void Rem_Un() => Binary("rem_un");
        public void And() => Binary("and");
        public void Or() => Binary("or");
        public void Xor() => Binary("xor");
        public void Shl() => Binary("shl");
        public void Shr() => Binary("shr");
        public void Shr_Un() => Binary("shr_un");
        public void Clt() => Binary("clt");
        public void Clt_Un() => Binary("clt_un");
        public void Ceq() => Binary("ceq");
        public void Cgt() => Binary("cgt");
        public void Cgt_Un() => Binary("cgt_un");

        // Branch

        public void Brtrue() => BranchIf(string.Empty);
        public void Brfalse() => BranchIf("!");
        public void Blt() => BranchCompare("clt");
        public void Blt_Un() => BranchCompare("clt_un");
        public void Ble() => BranchCompare("cle");
        public void Ble_Un() => BranchCompare("cle_un");
        public void Beq() => BranchCompare("ceq");
        public void Bge() => BranchCompare("cge");
        public void Bge_Un() => BranchCompare("cge_un");
        public void Bgt() => BranchCompare("cgt");
        public void Bgt_Un() => BranchCompare("cgt_un");
        public void Bne() => BranchCompare("cne");
        public void Bne_Un() => BranchCompare("cne_un");

        // Conversion

        public void Conv_I1() => Conversion(StackType.Int32, "i1");
        public void Conv_I2() => Conversion(StackType.Int32, "i2");
        public void Conv_I4() => Conversion(StackType.Int32, "i4");
        public void Conv_I8() => Conversion(StackType.Int64, "i8");
        public void Conv_I() => Conversion(StackType.NativeInt, "i");
        public void Conv_R4() => Conversion(StackType.F, "r4");
        public void Conv_R8() => Conversion(StackType.F, "r8");
        public void Conv_U1() => Conversion(StackType.Int32, "u1");
        public void Conv_U2() => Conversion(StackType.Int32, "u2");
        public void Conv_U4() => Conversion(StackType.Int32, "u4");
        public void Conv_U8() => Conversion(StackType.Int64, "u8");
        public void Conv_U() => Conversion(StackType.NativeInt, "u");
        public void Conv_Ovf_I1() => Conversion(StackType.Int32, "ovf_i1");
        public void Conv_Ovf_I2() => Conversion(StackType.Int32, "ovf_i2");
        public void Conv_Ovf_I4() => Conversion(StackType.Int32, "ovf_i4");
        public void Conv_Ovf_I8() => Conversion(StackType.Int64, "ovf_i8");
        public void Conv_Ovf_I() => Conversion(StackType.NativeInt, "ovf_i");
        public void Conv_Ovf_U1() => Conversion(StackType.Int32, "ovf_u1");
        public void Conv_Ovf_U2() => Conversion(StackType.Int32, "ovf_u2");
        public void Conv_Ovf_U4() => Conversion(StackType.Int32, "ovf_u4");
        public void Conv_Ovf_U8() => Conversion(StackType.Int64, "ovf_u8");
        public void Conv_Ovf_U() => Conversion(StackType.NativeInt, "ovf_u");
        public void Conv_Ovf_I1_Un() => Conversion(StackType.Int32, "ovf_i1_un");
        public void Conv_Ovf_I2_Un() => Conversion(StackType.Int32, "ovf_i2_un");
        public void Conv_Ovf_I4_Un() => Conversion(StackType.Int32, "ovf_i4_un");
        public void Conv_Ovf_I8_Un() => Conversion(StackType.Int64, "ovf_i8_un");
        public void Conv_Ovf_I_Un() => Conversion(StackType.NativeInt, "ovf_i_un");
        public void Conv_Ovf_U1_Un() => Conversion(StackType.Int32, "ovf_u1_un");
        public void Conv_Ovf_U2_Un() => Conversion(StackType.Int32, "ovf_u2_un");
        public void Conv_Ovf_U4_Un() => Conversion(StackType.Int32, "ovf_u4_un");
        public void Conv_Ovf_U8_Un() => Conversion(StackType.Int64, "ovf_u8_un");
        public void Conv_Ovf_U_Un() => Conversion(StackType.NativeInt, "ovf_u_un");

        // Ldind
        public void Ldind_I1() => Ldind(StackType.Int32, "i1");
        public void Ldind_I2() => Ldind(StackType.Int32, "i2");
        public void Ldind_I4() => Ldind(StackType.Int32, "i4");
        public void Ldind_I8() => Ldind(StackType.Int64, "i8");
        public void Ldind_R4() => Ldind(StackType.F, "r4");
        public void Ldind_R8() => Ldind(StackType.F, "r8");
        public void Ldind_I() => Ldind(StackType.NativeInt, "i");
        public void Ldind_Ref() => Ldind(StackType.O, "ref");
        public void Ldind_U1() => Ldind(StackType.Int32, "u1");
        public void Ldind_U2() => Ldind(StackType.Int32, "u2");
        public void Ldind_U4() => Ldind(StackType.Int32, "u4");

        // Stind
        public void Stind_I1() => Stind("i1");
        public void Stind_I2() => Stind("i2");
        public void Stind_I4() => Stind("i4");
        public void Stind_I8() => Stind("i8");
        public void Stind_R4() => Stind("r4");
        public void Stind_R8() => Stind("r8");
        public void Stind_I() => Stind("i");
        public void Stind_Ref() => Stind("ref");

        // Ldelem
        public void Ldelem_I1() => Ldelem(StackType.Int32, "i1");
        public void Ldelem_I2() => Ldelem(StackType.Int32, "i2");
        public void Ldelem_I4() => Ldelem(StackType.Int32, "i4");
        public void Ldelem_I8() => Ldelem(StackType.Int64, "i8");
        public void Ldelem_R4() => Ldelem(StackType.F, "r4");
        public void Ldelem_R8() => Ldelem(StackType.F, "r8");
        public void Ldelem_I() => Ldelem(StackType.NativeInt, "i");
        public void Ldelem_Ref() => Ldelem(StackType.O, "ref");
        public void Ldelem_U1() => Ldelem(StackType.Int32, "u1");
        public void Ldelem_U2() => Ldelem(StackType.Int32, "u2");
        public void Ldelem_U4() => Ldelem(StackType.Int32, "u4");

        // Stelem
        public void Stelem_I1() => Stelem("i1");
        public void Stelem_I2() => Stelem("i2");
        public void Stelem_I4() => Stelem("i4");
        public void Stelem_I8() => Stelem("i8");
        public void Stelem_R4() => Stelem("r4");
        public void Stelem_R8() => Stelem("r8");
        public void Stelem_I() => Stelem("i");
        public void Stelem_Ref() => Stelem("ref");

        public void Ldarg()
        {
            var param = Op.GetParameter(Method.Parameters.ToList());
            var paramName = param.IsHiddenThisParameter ? "_this" : param.ToString();
            Stack.Push(TypeUtils.GetStackType(param.Type), $"::natsu::stack_from({paramName})");
        }

        public void Starg()
        {
            var param = Op.GetParameter(Method.Parameters.ToList());
            var paramName = param.IsHiddenThisParameter ? "_this" : param.ToString();
            var value = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"{paramName} = {CastExpression(param.Type, value)};");
        }

        public void Ldarga()
        {
            var param = Op.GetParameter(Method.Parameters.ToList());
            var paramName = param.IsHiddenThisParameter ? "_this" : param.ToString();
            Stack.Push(StackType.Ref, $"::natsu::ops::ref({paramName})");
        }

        public void LdcI4()
        {
            var value = Op.GetLdcI4Value();
            Stack.Push(StackType.Int32, $"::natsu::stack::int32({TypeUtils.LiteralConstant(value)})");
        }

        public void Ldc_I8()
        {
            var value = (long)Op.Operand;
            Stack.Push(StackType.Int64, $"::natsu::stack::int64({TypeUtils.LiteralConstant(value)})");
        }

        public void Ldc_R4()
        {
            var value = (float)Op.Operand;
            Stack.Push(StackType.F, $"::natsu::stack::F({TypeUtils.LiteralConstant(value)})");
        }

        public void Ldc_R8()
        {
            var value = (double)Op.Operand;
            Stack.Push(StackType.F, $"::natsu::stack::F({TypeUtils.LiteralConstant(value)})");
        }

        public void Call()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var para = new List<(TypeSig destType, StackEntry src)>();
            var parasCount = method.Params.Count;
            for (int i = parasCount - 1; i >= 0; i--)
                para.Add((method.Params[i], Stack.Pop()));

            if (method.HasThis)
                para.Add((TypeUtils.ThisType(member.DeclaringType), Stack.Pop()));

            if (Method.FullName.Contains("Log2SoftwareFallback"))
                ;
            var gen = (member as MethodSpec)?.GenericInstMethodSig;
            para.Reverse();
            string expr;
            if (gen != null)
            {
                var genArgs = gen.GenericArguments;
                expr = $"{TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member)}<{string.Join(", ", gen.GenericArguments.Select(x => TypeUtils.EscapeTypeName(x)))}>({string.Join(", ", para.Select((x, i) => CastExpression(x.destType, x.src, genArgs)))})";
            }
            else
            {
                expr = $"{ TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src)))})";
            }

            var stackType = TypeUtils.GetStackType(method.RetType);
            if (stackType == StackType.Void)
                Stack.Push(stackType, expr);
            else
                Stack.Push(stackType, $"::natsu::stack_from({expr})");
        }

        public void Callvirt()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var para = new List<(TypeSig destType, StackEntry src)>();
            var parasCount = method.Params.Count;
            for (int i = parasCount - 1; i >= 0; i--)
                para.Add((method.Params[i], Stack.Pop()));
            para.Add((TypeUtils.ThisType(member.DeclaringType), Stack.Pop()));

            para.Reverse();
            string expr;
            if (Stack.Constrained != null)
            {
                Stack.Push(StackType.O, $"::natsu::ops::box(*::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(new ByRefSig(Stack.Constrained.ToTypeSig()))}>({para[0].src.Expression}))");
                para[0] = (para[0].destType, Stack.Pop());

                expr = $"{para[0].src.Expression}.header().template vtable_as<{TypeUtils.EscapeTypeName(member.DeclaringType)}::VTable>()->{TypeUtils.EscapeMethodName(member)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src)))})";

                Stack.Constrained = null;
            }
            else
            {
                Writer.Ident(Ident).WriteLine($"::natsu::check_null_obj_ref({para[0].src.Expression});");
                expr = $"{para[0].src.Expression}.header().template vtable_as<{TypeUtils.EscapeTypeName(member.DeclaringType)}::VTable>()->{TypeUtils.EscapeMethodName(member)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src)))})";
            }

            var stackType = TypeUtils.GetStackType(method.RetType);
            if (stackType == StackType.Void)
                Stack.Push(stackType, expr);
            else
                Stack.Push(stackType, $"::natsu::stack_from({expr})");
        }

        private static string CastExpression(TypeSig destType, StackEntry src, IList<TypeSig> genArgs = null)
        {
            return $"::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(destType, hasGen: 1, genArgs: genArgs)}>({src.Expression})";
        }

        public void Nop()
        {
            Writer.Ident(Ident).WriteLine("::natsu::nop();");
        }

        public void Ret()
        {
            if (Method.HasReturnType)
            {
                var value = Stack.Pop();
                Writer.Ident(Ident).WriteLine($"return {CastExpression(Method.ReturnType, value)};");
            }
            else
            {
                Writer.Ident(Ident).WriteLine("return;");
            }
        }

        public void Throw()
        {
            var v1 = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::throw_({v1.Expression});");
        }

        public void Ldloc()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            Stack.Push(TypeUtils.GetStackType(local.Type), $"::natsu::stack_from(_l{local.Index})");
        }

        public void Ldloca()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            Stack.Push(StackType.Ref, $"::natsu::ops::ref(_l{local.Index})");
        }

        public void Stloc()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            var value = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"_l{local.Index} = {CastExpression(local.Type, value)};");
        }

        public void Br()
        {
            var nextOp = (Instruction)Op.Operand;
            BranchUnconditional(Ident, nextOp);
        }

        public void Unary(string op)
        {
            var v1 = Stack.Pop();
            Stack.Push(StackType.Int32, $"::natsu::ops::{op}({v1.Expression})");
        }

        public void Binary(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            Stack.Push(StackType.Int32, $"::natsu::ops::{op}({v1.Expression}, {v2.Expression})");
        }

        private void BranchUnconditional(int ident, Instruction op)
        {
            Writer.Ident(Ident).WriteLine($"goto {ILUtils.GetLabel(Method, op, Block)};");
        }

        private void BranchCompare(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var nextOp = (Instruction)Op.Operand;
            Writer.Ident(Ident).WriteLine($"if (::natsu::ops::{op}({v1.Expression}, {v2.Expression}).istrue())");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetLabel(Method, nextOp, Block)};");
            Writer.Ident(Ident).WriteLine("else");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
        }

        private void BranchIf(string op)
        {
            var v1 = Stack.Pop();
            var nextOp = (Instruction)Op.Operand;
            Writer.Ident(Ident).WriteLine($"if ({op}{v1.Expression}.istrue())");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetLabel(Method, nextOp, Block)};");
            Writer.Ident(Ident).WriteLine("else");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
        }

        public void Ldstr()
        {
            var value = (string)Op.Operand;
            Stack.Push(StackType.O, $"::natsu::stack_from(::natsu::load_string(uR\"NS({value})NS\"sv))");
        }

        public void Ldsfld()
        {
            var field = (IField)Op.Operand;
            string expr = Method.IsStaticConstructor && Method.DeclaringType == field.DeclaringType
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::value." + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            Stack.Push(TypeUtils.GetStackType(fieldType), $"::natsu::stack_from({expr})");
        }

        public void Ldfld()
        {
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            string expr = $"::natsu::ops::access<{TypeUtils.EscapeVariableTypeName(thisType)}>({target.Expression})->" + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            Stack.Push(TypeUtils.GetStackType(fieldType), $"::natsu::stack_from({expr})");
        }

        public void Ldflda()
        {
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            string expr = $"::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(thisType)}>({target.Expression})->" + TypeUtils.EscapeIdentifier(field.Name);
            Stack.Push(StackType.Ref, $"::natsu::ops::ref({expr})");
        }

        public void Ldsflda()
        {
            var field = (IField)Op.Operand;
            string expr = Method.IsStaticConstructor && Method.DeclaringType == field.DeclaringType
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::value." + TypeUtils.EscapeIdentifier(field.Name);
            Stack.Push(StackType.Ref, $"::natsu::ops::ref({expr})");
        }

        public void Stsfld()
        {
            var value = Stack.Pop();
            var field = (IField)Op.Operand;
            string expr = Method.IsStaticConstructor && Method.DeclaringType == field.DeclaringType
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::value." + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            Writer.Ident(Ident).WriteLine($"{expr} = ::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(fieldType)}>({value.Expression});");
        }

        public void Stfld()
        {
            var value = Stack.Pop();
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            string expr = $"::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(thisType)}>({target.Expression})->" + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            Writer.Ident(Ident).WriteLine($"{expr} = ::natsu::stack_to<{TypeUtils.EscapeVariableTypeName(fieldType)}>({value.Expression});");
        }

        public void Newarr()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var len = Stack.Pop();
            Stack.Push(StackType.O, $"::natsu::stack_from(::natsu::gc_new_array<{TypeUtils.EscapeTypeName(type)}>({len.Expression}))");
        }

        public void Initobj()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var addr = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::initobj<{TypeUtils.EscapeVariableTypeName(type)}>({addr.Expression});");
        }

        public void Newobj()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var para = new List<(TypeSig destType, StackEntry src)>();
            var parasCount = method.Params.Count;
            for (int i = parasCount - 1; i >= 0; i--)
                para.Add((method.Params[i], Stack.Pop()));

            para.Reverse();
            var genSig = member.DeclaringType.TryGetGenericInstSig();
            var expr = $"::natsu::ops::newobj<{TypeUtils.EscapeTypeName(member.DeclaringType)}>({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, genSig?.GenericArguments)))})";
            Stack.Push(StackType.O, expr);
        }

        public void Ldobj()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var addr = Stack.Pop();
            Stack.Push(StackType.O, $"::natsu::ops::ldobj<{TypeUtils.EscapeVariableTypeName(type)}>({addr.Expression})");
        }

        public void Stobj()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var src = Stack.Pop();
            var dest = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::stobj<{TypeUtils.EscapeVariableTypeName(type)}>({src.Expression}, {dest.Expression});");
        }

        public void Ldtoken()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            Stack.Push(StackType.ValueType, $"::natsu::ops::ldtoken_type<{TypeUtils.EscapeTypeName(type)}>()");
        }

        public void Isinst()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(StackType.O, $"::natsu::ops::isinst<{TypeUtils.EscapeTypeName(type)}>({obj.Expression})");
        }

        public void Unbox_Any()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(StackType.O, $"::natsu::ops::unbox_any<{TypeUtils.EscapeTypeName(type)}>({obj.Expression})");
        }

        public void Unbox()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(StackType.Ref, $"::natsu::ops::unbox<{TypeUtils.EscapeTypeName(type)}>({obj.Expression})");
        }

        public void Ldlen()
        {
            var target = Stack.Pop();
            Stack.Push(StackType.NativeInt, $"::natsu::ops::ldlen({target.Expression})");
        }

        private void Conversion(StackType stackType, string type)
        {
            var value = Stack.Pop();
            Stack.Push(stackType, $"::natsu::ops::conv_{type}({value.Expression})");
        }

        private void Ldind(StackType stackType, string type)
        {
            var addr = Stack.Pop();
            Stack.Push(stackType, $"::natsu::ops::ldind_{type}({addr.Expression})");
        }

        private void Stind(string type)
        {
            var value = Stack.Pop();
            var addr = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::stind_{type}({addr.Expression}, {value.Expression});");
        }

        private void Ldelem(StackType stackType, string type)
        {
            var index = Stack.Pop();
            var array = Stack.Pop();
            Stack.Push(stackType, $"::natsu::ops::ldelem_{type}({array.Expression}, {index.Expression})");
        }

        private void Stelem(string type)
        {
            var value = Stack.Pop();
            var index = Stack.Pop();
            var array = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::stelem_{type}({array.Expression}, {index.Expression}, {value.Expression});");
        }

        public void Ldelema()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var index = Stack.Pop();
            var array = Stack.Pop();
            Stack.Push(StackType.Ref, $"::natsu::ops::ldelema<{TypeUtils.EscapeTypeName(type)}>({array.Expression}, {index.Expression})");
        }

        public void Box()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var value = Stack.Pop();
            Stack.Push(StackType.O, $"::natsu::ops::box({CastExpression(type.ToTypeSig(), value)})");
        }

        public void Ldnull()
        {
            Stack.Push(StackType.O, $"::natsu::stack::null");
        }

        public void Dup()
        {
            var value = Stack.Peek();
            Stack.Push(value);
        }

        public void Switch()
        {
            var instructions = (Instruction[])Op.Operand;
            var v1 = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"switch ({v1.Expression}.value_)");
            Writer.Ident(Ident).WriteLine("{");
            for (int i = 0; i < instructions.Length; i++)
            {
                Writer.Ident(Ident + 1).WriteLine($"case {i}:");
                Writer.Ident(Ident + 2).WriteLine($"goto {ILUtils.GetLabel(Method, instructions[i], Block)};");
            }
            Writer.Ident(Ident + 1).WriteLine("default:");
            Writer.Ident(Ident + 2).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
            Writer.Ident(Ident).WriteLine("}");
        }
    }
}
