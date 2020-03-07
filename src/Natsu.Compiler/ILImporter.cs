using System;
using System.Collections.Generic;
using System.Diagnostics;
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
        private BlockGraph _blockGraph = new BlockGraph();
        private readonly TextWriter _writer;
        private readonly int _ident;
        private readonly MethodDef _method;
        private readonly CorLibTypes _corLibTypes;
        private Dictionary<ExceptionHandler, ExceptionHandlerContext> _exceptions = new Dictionary<ExceptionHandler, ExceptionHandlerContext>();
        private int _paramIndex;
        private int _blockId;
        private int _nextSpillSlot = 0;

        public List<string> UserStrings { get; set; }
        public string ModuleName { get; set; }

        public ILImporter(CorLibTypes corLibTypes, MethodDef method, TextWriter writer, int ident)
        {
            _corLibTypes = corLibTypes;
            _method = method;
            _writer = writer;
            _ident = ident;
        }

        public void ImportNormalBlocks()
        {
            _headBlock = ImportBlocks(_method.Body.Instructions, _method.Body.Instructions.FirstOrDefault());
        }

        public void ImportExceptionBlocks()
        {
            foreach (var handler in _method.Body.ExceptionHandlers)
            {
                var ctx = new ExceptionHandlerContext
                {
                    Handler = handler,
                    HeadBlock = ImportBlocks(_method.Body.Instructions, handler.HandlerStart)
                };

                _exceptions.Add(handler, ctx);
            }
        }

        Instruction PrevInst(Instruction inst)
        {
            var instructions = _method.Body.Instructions;
            var index = instructions.IndexOf(inst);
            if (index == 0)
                return null;
            return instructions[index - 1];
        }

        private BasicBlock ImportBlocks(IList<Instruction> instructions, Instruction firstInst)
        {
            Instruction NextInst(Instruction inst)
            {
                var index = instructions.IndexOf(inst);
                if (index < instructions.Count - 1)
                    return instructions[index + 1];
                return null;
            }

            BasicBlock ImportBlock(BasicBlock parent, Instruction inst)
            {
                if (inst == null) return null;

                var block = new BasicBlock { Id = _blockId++, Parent = parent };
                _blockGraph.Blocks.Add(inst, block);
                bool conti = true;

                void AddNext(Instruction next)
                {
                    if (!_blockGraph.Blocks.TryGetValue(next, out var nextBlock))
                        nextBlock = ImportBlock(block, next);

                    if (nextBlock != null)
                        block.Next.Add(nextBlock);
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
                        case Code.Leave:
                        case Code.Leave_S:
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
                        case Code.Endfinally:
                        case Code.Endfilter:
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

            if (firstInst != null)
                return ImportBlock(null, firstInst);
            else
                return new BasicBlock { Id = 0 };
        }

        internal void Gencode()
        {
            var visited = new HashSet<BasicBlock>();
            var spills = new List<SpillSlot>();
            VisitBlock(_ident, _headBlock, visited, spills);
            WriteSpills(spills, _writer, _ident);
            visited.Clear();
            VisitBlockText(_headBlock, _writer, visited);
        }

        private void WriteSpills(List<SpillSlot> spills, TextWriter writer, int ident)
        {
            var spillsName = new HashSet<string>();
            // spills
            foreach (var spill in spills.Distinct())
            {
                if (spillsName.Add(spill.Name))
                    writer.Ident(ident).WriteLine($"{TypeUtils.EscapeStackTypeName(spill.Entry.Type)} {spill.Name};");
            }
        }

        private void VisitBlockText(BasicBlock block, TextWriter writer, HashSet<BasicBlock> visited)
        {
            var blocks = new List<BasicBlock>();
            void AddBlock(BasicBlock headBlock)
            {
                visited.Add(headBlock);
                blocks.Add(headBlock);
                foreach (var next in headBlock.Next)
                {
                    if (!visited.Contains(next))
                        AddBlock(next);
                }
            }

            AddBlock(block);
            foreach (var cntBlock in blocks.Where(x => x.Instructions.Any())
                .OrderBy(x => x.Instructions[0].Offset))
            {
                writer.Write(cntBlock.Text);
            }
        }

        private void VisitBlock(int ident, BasicBlock block, HashSet<BasicBlock> visited, List<SpillSlot> spills, StringWriter writer = null, EvaluationStack stack = null)
        {
            visited.Add(block);

            writer = writer ?? new StringWriter();
            stack = stack ?? new EvaluationStack(writer, ident, _paramIndex);
            writer.WriteLine(ILUtils.GetLabel(_method, block.Id) + ":");

            // import spills
            if (block.Parent != null)
            {
                foreach (var spill in Enumerable.Reverse(block.Parent.Spills))
                    stack.Push(spill.Entry.Type, spill.Name);
            }

            var instLines = new List<string>();
            foreach (var op in block.Instructions)
            {
                var instW = new StringWriter();
                stack.SetWriter(instW);

                var tryEnter = (from e in _exceptions
                                where !e.Value.EnterProcessed && e.Key.TryStart == op
                                select e).FirstOrDefault();
                if (tryEnter.Key != null)
                {
                    EvaluationStack tryEnterStack = null;
                    if (tryEnter.Key.HandlerType == ExceptionHandlerType.Finally)
                    {
                        if (_method.FullName.Contains("Concat"))
                            ;
                        instW.Ident(ident).WriteLine("{");
                        ident += 1;
                        stack.Ident = ident;
                        var captures = new List<string>();
                        // locals
                        foreach (var local in _method.Body.Variables)
                            captures.Add("&" + TypeUtils.GetLocalName(local, _method));
                        foreach(var param in _method.Parameters)
                        {
                            var paramName = param.IsHiddenThisParameter ? "_this" : param.ToString();
                            captures.Add("&" + paramName);
                        }

                        instW.Ident(ident).WriteLine($"auto _scope_finally = natsu::make_finally([{string.Join(", ", captures)}]{{");
                        var finallySpills = new List<SpillSlot>();
                        VisitBlock(ident + 1, tryEnter.Value.HeadBlock, new HashSet<BasicBlock>(), finallySpills, stack: tryEnterStack);
                        WriteSpills(finallySpills, instW, ident + 1);
                        VisitBlockText(tryEnter.Value.HeadBlock, instW, visited);
                        instW.Ident(ident).WriteLine("});");
                        tryEnter.Value.EnterProcessed = true;
                    }
                    else if (tryEnter.Key.HandlerType == ExceptionHandlerType.Catch)
                    {

                    }
                }

                WriteInstruction(instW, op, stack, ident, block);

                var tryExit = (from e in _exceptions
                               where !e.Value.ExitProcessed && PrevInst(e.Key.TryEnd) == op
                               select e).FirstOrDefault();
                if (tryExit.Key != null)
                {
                    if (tryExit.Key.HandlerType == ExceptionHandlerType.Finally)
                    {
                        ident -= 1;
                        stack.Ident = ident;
                        instW.Ident(ident).WriteLine("}");
                        tryExit.Value.ExitProcessed = true;
                    }
                    else if (tryExit.Key.HandlerType == ExceptionHandlerType.Catch)
                    {

                    }
                }

                instLines.Add(instW.ToString());
            }

            foreach (var instLine in instLines.Take(instLines.Count - 1))
                writer.Write(instLine);

            int slotIndex = 0;
            // export spills
            while (block.Next.Count != 0 && !stack.Empty)
            {
                var spill = AddSpillSlot(slotIndex++, stack.Pop(), block, spills);
                writer.Ident(ident).WriteLine($"{spill.Name} = {spill.Entry.Expression};");
                block.Spills.Add(spill);
            }

            writer.Write(instLines.Last());

            block.Text = writer.ToString();
            _paramIndex = stack.ParamIndex;

            foreach (var next in block.Next)
            {
                if (!visited.Contains(next))
                    VisitBlock(ident, next, visited, spills);
            }
        }

        private SpillSlot AddSpillSlot(int index, StackEntry stackEntry, BasicBlock block, List<SpillSlot> spills)
        {
            if (block.Next != null)
            {
                foreach (var slot in spills)
                {
                    if (slot.Next != block.Next && slot.Index == index)
                    {
                        foreach (var next in slot.Next)
                        {
                            if (block.Next.Contains(next))
                            {
                                var newSlot = new SpillSlot { Index = slot.Index, Name = slot.Name, Entry = stackEntry, Next = block.Next };
                                spills.Add(newSlot);
                                return newSlot;
                            }
                        }
                    }
                }
            }

            {
                var slot = new SpillSlot { Index = index, Name = "_s" + _nextSpillSlot++.ToString(), Entry = stackEntry, Next = block.Next };
                spills.Add(slot);
                return slot;
            }
        }

        private void WriteInstruction(TextWriter writer, Instruction op, EvaluationStack stack, int ident, BasicBlock block)
        {
            var emitter = new OpEmitter { CorLibTypes = _corLibTypes, ModuleName = ModuleName, UserStrings = UserStrings, Method = _method, Op = op, Stack = stack, Ident = ident, Block = block, Writer = writer };
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
            else if (op.IsLeave())
                emitter.Leave();
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
                    case Code.Endfinally:
                        emitter.Endfinally();
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
                    case Code.Add_Ovf:
                        emitter.Add_Ovf();
                        break;
                    case Code.Sub:
                        emitter.Sub();
                        break;
                    case Code.Sub_Ovf:
                        emitter.Sub_Ovf();
                        break;
                    case Code.Mul:
                        emitter.Mul();
                        break;
                    case Code.Mul_Ovf:
                        emitter.Mul_Ovf();
                        break;
                    case Code.Mul_Ovf_Un:
                        emitter.Mul_Ovf_Un();
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
                    case Code.Stind_I:
                        emitter.Stind_I();
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
                    case Code.Ldelem:
                        emitter.Ldelem();
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
                    case Code.Stelem:
                        emitter.Stelem();
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
                    case Code.Castclass:
                        emitter.Castclass();
                        break;
                    case Code.Ldlen:
                        emitter.Ldlen();
                        break;
                    case Code.Localloc:
                        emitter.Localloc();
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
                    case Code.Ldftn:
                        emitter.Ldftn();
                        break;
                    case Code.Ldvirtftn:
                        emitter.Ldvirtftn();
                        break;
                    case Code.Pop:
                        emitter.Pop();
                        break;
                    case Code.Sizeof:
                        emitter.Sizeof();
                        break;
                    case Code.Constrained:
                        stack.Constrained = (ITypeDefOrRef)op.Operand;
                        break;
                    case Code.Volatile:
                        stack.Volatile = true;
                        break;
                    case Code.Readonly:
                        break;
                    default:
                        throw new NotSupportedException(op.OpCode.Code.ToString());
                }
            }
        }

        private class ExceptionHandlerContext
        {
            public ExceptionHandler Handler;
            public bool EnterProcessed;
            public bool ExitProcessed;
            public BasicBlock HeadBlock;
        }
    }

    class OpEmitter
    {
        public Instruction Op { get; set; }
        public EvaluationStack Stack { get; set; }
        public int Ident { get; set; }
        public BasicBlock Block { get; set; }
        public MethodDef Method { get; set; }
        public TextWriter Writer { get; set; }
        public string ModuleName { get; set; }
        public List<string> UserStrings { get; set; }
        public CorLibTypes CorLibTypes { get; set; }

        // Unary

        public void Neg() => Unary("-");
        public void Not() => Unary("~");

        // Binary

        public void Add() => Binary("+");
        public void Add_Ovf() => Binary_Ovf("+");
        public void Sub() => Binary("-");
        public void Sub_Ovf() => Binary_Ovf("-");
        public void Mul() => Binary("*");
        public void Mul_Ovf() => Binary_Ovf("*");
        public void Mul_Ovf_Un() => Binary_Ovf_Un("*");
        public void Div() => Binary("/");
        public void Div_Un() => Binary_Un("/");
        public void Rem() => Binary("%");
        public void Rem_Un() => Binary_Un("%");
        public void And() => Binary("&");
        public void Or() => Binary("|");
        public void Xor() => Binary("^");
        public void Shl() => Binary("<<");
        public void Shr() => Binary(">>");
        public void Shr_Un() => Binary_Un(">>");
        public void Clt() => Compare("<");
        public void Clt_Un() => Compare_Un("<");
        public void Ceq() => Compare("==");
        public void Cgt() => Compare(">");
        public void Cgt_Un() => Compare_Un(">");

        // Branch

        public void Brtrue() => BranchIf(string.Empty);
        public void Brfalse() => BranchIf("!");
        public void Blt() => BranchCompare("<");
        public void Blt_Un() => BranchCompare_Un("<");
        public void Ble() => BranchCompare("<=");
        public void Ble_Un() => BranchCompare_Un("<=");
        public void Beq() => BranchCompare("==");
        public void Bge() => BranchCompare(">=");
        public void Bge_Un() => BranchCompare_Un(">=");
        public void Bgt() => BranchCompare(">");
        public void Bgt_Un() => BranchCompare_Un(">");
        public void Bne() => BranchCompare("!=");
        public void Bne_Un() => BranchCompare_Un("!=");

        // Conversion

        public void Conv_I1() => Conversion(CorLibTypes.SByte, "i1");
        public void Conv_I2() => Conversion(CorLibTypes.Int16, "i2");
        public void Conv_I4() => Conversion(CorLibTypes.Int32, "i4");
        public void Conv_I8() => Conversion(CorLibTypes.Int64, "i8");
        public void Conv_I() => Conversion(CorLibTypes.IntPtr, "i");
        public void Conv_R4() => Conversion(CorLibTypes.Single, "r4");
        public void Conv_R8() => Conversion(CorLibTypes.Double, "r8");
        public void Conv_U1() => Conversion(CorLibTypes.Byte, "u1");
        public void Conv_U2() => Conversion(CorLibTypes.UInt16, "u2");
        public void Conv_U4() => Conversion(CorLibTypes.UInt32, "u4");
        public void Conv_U8() => Conversion(CorLibTypes.UInt64, "u8");
        public void Conv_U() => Conversion(CorLibTypes.UIntPtr, "u");
        public void Conv_Ovf_I1() => Conversion(CorLibTypes.SByte, "ovf_i1");
        public void Conv_Ovf_I2() => Conversion(CorLibTypes.Int16, "ovf_i2");
        public void Conv_Ovf_I4() => Conversion(CorLibTypes.Int32, "ovf_i4");
        public void Conv_Ovf_I8() => Conversion(CorLibTypes.Int64, "ovf_i8");
        public void Conv_Ovf_I() => Conversion(CorLibTypes.IntPtr, "ovf_i");
        public void Conv_Ovf_U1() => Conversion(CorLibTypes.Byte, "ovf_u1");
        public void Conv_Ovf_U2() => Conversion(CorLibTypes.UInt16, "ovf_u2");
        public void Conv_Ovf_U4() => Conversion(CorLibTypes.UInt32, "ovf_u4");
        public void Conv_Ovf_U8() => Conversion(CorLibTypes.UInt64, "ovf_u8");
        public void Conv_Ovf_U() => Conversion(CorLibTypes.UIntPtr, "ovf_u");
        public void Conv_Ovf_I1_Un() => Conversion(CorLibTypes.SByte, "ovf_i1_un");
        public void Conv_Ovf_I2_Un() => Conversion(CorLibTypes.Int16, "ovf_i2_un");
        public void Conv_Ovf_I4_Un() => Conversion(CorLibTypes.Int32, "ovf_i4_un");
        public void Conv_Ovf_I8_Un() => Conversion(CorLibTypes.Int64, "ovf_i8_un");
        public void Conv_Ovf_I_Un() => Conversion(CorLibTypes.IntPtr, "ovf_i_un");
        public void Conv_Ovf_U1_Un() => Conversion(CorLibTypes.Byte, "ovf_u1_un");
        public void Conv_Ovf_U2_Un() => Conversion(CorLibTypes.UInt16, "ovf_u2_un");
        public void Conv_Ovf_U4_Un() => Conversion(CorLibTypes.UInt32, "ovf_u4_un");
        public void Conv_Ovf_U8_Un() => Conversion(CorLibTypes.UInt64, "ovf_u8_un");
        public void Conv_Ovf_U_Un() => Conversion(CorLibTypes.UIntPtr, "ovf_u_un");

        // Ldind
        public void Ldind_I1() => Ldind(CorLibTypes.SByte);
        public void Ldind_I2() => Ldind(CorLibTypes.Int16);
        public void Ldind_I4() => Ldind(CorLibTypes.Int32);
        public void Ldind_I8() => Ldind(CorLibTypes.Int64);
        public void Ldind_R4() => Ldind(CorLibTypes.Single);
        public void Ldind_R8() => Ldind(CorLibTypes.Double);
        public void Ldind_I() => Ldind(CorLibTypes.IntPtr);
        public void Ldind_Ref()
        {
            var addr = Stack.Pop();
            Stack.Push(addr.Type.TypeSig.Next, $"(*{addr.Expression})");
        }

        public void Ldind_U1() => Ldind(CorLibTypes.Byte);
        public void Ldind_U2() => Ldind(CorLibTypes.UInt16);
        public void Ldind_U4() => Ldind(CorLibTypes.UInt32);

        // Stind
        public void Stind_I1() => Stind(CorLibTypes.SByte);
        public void Stind_I2() => Stind(CorLibTypes.Int16);
        public void Stind_I4() => Stind(CorLibTypes.Int32);
        public void Stind_I8() => Stind(CorLibTypes.Int64);
        public void Stind_R4() => Stind(CorLibTypes.Single);
        public void Stind_R8() => Stind(CorLibTypes.Double);
        public void Stind_I() => Stind(CorLibTypes.IntPtr);
        public void Stind_Ref()
        {
            var value = Stack.Pop();
            var addr = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"*{addr.Expression} = {value.Expression};");
        }

        // Ldelem
        public void Ldelem_I1() => Ldelem(CorLibTypes.SByte, "i1");
        public void Ldelem_I2() => Ldelem(CorLibTypes.Int16, "i2");
        public void Ldelem_I4() => Ldelem(CorLibTypes.Int32, "i4");
        public void Ldelem_I8() => Ldelem(CorLibTypes.Int64, "i8");
        public void Ldelem_R4() => Ldelem(CorLibTypes.Single, "r4");
        public void Ldelem_R8() => Ldelem(CorLibTypes.Double, "r8");
        public void Ldelem_I() => Ldelem(CorLibTypes.IntPtr, "i");
        public void Ldelem_Ref()
        {
            var index = Stack.Pop();
            var array = Stack.Pop();
            var elemType = array.Type.TypeSig.Next;
            Stack.Push(elemType.ToTypeDefOrRef(), $"{array.Expression}->at({index.Expression})");
        }
        public void Ldelem_U1() => Ldelem(CorLibTypes.Byte, "u1");
        public void Ldelem_U2() => Ldelem(CorLibTypes.UInt16, "u2");
        public void Ldelem_U4() => Ldelem(CorLibTypes.UInt32, "u4");

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
            Stack.Push(TypeUtils.GetStackType(param.Type), $"{paramName}", computed: false);
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
            Stack.Push(TypeUtils.GetStackType(new ByRefSig(param.Type)), $"::natsu::ops::ref({paramName})", computed: true);
        }

        public void LdcI4()
        {
            var value = Op.GetLdcI4Value();
            Stack.Push(CorLibTypes.Int32, $"{TypeUtils.LiteralConstant(value)}", computed: true);
        }

        public void Ldc_I8()
        {
            var value = (long)Op.Operand;
            Stack.Push(CorLibTypes.Int64, $"{TypeUtils.LiteralConstant(value)}", computed: true);
        }

        public void Ldc_R4()
        {
            var value = (float)Op.Operand;
            Stack.Push(CorLibTypes.Single, $"{TypeUtils.LiteralConstant(value)}", computed: true);
        }

        public void Ldc_R8()
        {
            var value = (double)Op.Operand;
            Stack.Push(CorLibTypes.Double, $"{TypeUtils.LiteralConstant(value)}", computed: true);
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

            var tGen = (member.DeclaringType as TypeSpec)?.TryGetGenericInstSig()?.GenericArguments.ToList()
                ?? new List<TypeSig>();
            var gen = (member as MethodSpec)?.GenericInstMethodSig;
            para.Reverse();
            string expr;
            if (gen != null)
            {
                var genArgs = gen.GenericArguments;
                tGen.AddRange(genArgs);
                expr = $"{TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}<{string.Join(", ", gen.GenericArguments.Select(x => TypeUtils.EscapeTypeName(x, cppBasicType: true)))}>({string.Join(", ", para.Select((x, i) => CastExpression(x.destType, x.src, genArgs)))})";
            }
            else
            {
                expr = $"{ TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, tGen)))})";
            }

            var stackType = TypeUtils.GetStackType(method.RetType, tGen);
            if (stackType.Code == StackTypeCode.Void)
                Stack.Push(stackType, expr);
            else
                Stack.Push(stackType, $"{expr}");
        }

        public void Callvirt()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var para = new List<(TypeSig destType, StackEntry src)>();
            var parasCount = method.Params.Count;
            for (int i = parasCount - 1; i >= 0; i--)
                para.Add((method.Params[i], Stack.Pop()));
            Stack.Compute();
            para.Add((TypeUtils.ThisType(member.DeclaringType), Stack.Pop()));

            var tGen = (member.DeclaringType as TypeSpec)?.TryGetGenericInstSig()?.GenericArguments.ToList()
                ?? new List<TypeSig>();
            var gen = (member as MethodSpec)?.GenericInstMethodSig;
            para.Reverse();
            string expr;
            if (Stack.Constrained != null)
            {
                var thisTypeCode = para[0].src.Type.Code;
                if (thisTypeCode == StackTypeCode.O || thisTypeCode == StackTypeCode.Runtime)
                {
                    Stack.Push(para[0].destType.ToTypeDefOrRef(), $"::natsu::ops::box(*{para[0].src.Expression})");
                    para[0] = (para[0].destType, Stack.Pop());

                    expr = $"{ TypeUtils.EscapeTypeName(Stack.Constrained)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, tGen)))})";
                }
                else
                {
                    expr = $"{ TypeUtils.EscapeTypeName(Stack.Constrained)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, tGen)))})";
                }

                Stack.Constrained = null;
            }
            else
            {
                expr = $"{para[0].src.Expression}.header().template vtable_as<typename {TypeUtils.EscapeTypeName(member.DeclaringType)}::VTable>()->{TypeUtils.EscapeMethodName(member)}({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, tGen)))})";
            }

            var stackType = TypeUtils.GetStackType(method.RetType, tGen);
            if (stackType.Code == StackTypeCode.Void)
                Stack.Push(stackType, expr);
            else
                Stack.Push(stackType, $"{expr}");
        }

        private static string CastExpression(TypeSig destType, StackEntry src, IList<TypeSig> genArgs = null)
        {
            if (src.Type.Name == "std::nullptr_t")
            {
                return $"{TypeUtils.EscapeVariableTypeName(destType, genArgs: genArgs)}(nullptr)";
            }
            else if (TypeUtils.IsSameType(destType, src.Type)
                || destType.ElementType == src.Type.TypeSig.ElementType)
            {
                return $"{src.Expression}";
            }
            else if (destType.ElementType == ElementType.Ptr &&
                src.Type.Code == StackTypeCode.Ref)
            {
                return $"{src.Expression}.get()";
            }
            else if (!TypeUtils.IsByRef(destType) &&
                src.Type.Code == StackTypeCode.Ref)
            {
                return $"*{src.Expression}";
            }
            else
            {
                if (destType.IsValueType)
                {
                    if (!destType.IsPrimitive)
                    {
                        return $"::natsu::ops::cast<{TypeUtils.EscapeVariableTypeName(destType, genArgs: genArgs)}>({src.Expression})";
                    }
                    else
                    {
                        return $"static_cast<{TypeUtils.EscapeVariableTypeName(destType, genArgs: genArgs)}>({src.Expression})";
                    }
                }
                else if (src.Type.Code != StackTypeCode.Ref)
                {
                    return $"{TypeUtils.EscapeVariableTypeName(destType, genArgs: genArgs)}({src.Expression})";
                }
                else
                {
                    return src.Expression;
                }
            }
        }

        private static string MakeAccessExpression(StackEntry src)
        {
            switch (src.Type.Code)
            {
                case StackTypeCode.Int32:
                case StackTypeCode.Int64:
                case StackTypeCode.NativeInt:
                case StackTypeCode.F:
                case StackTypeCode.ValueType:
                    return $"{src.Expression}.";
                case StackTypeCode.O:
                case StackTypeCode.Ref:
                    return $"{src.Expression}->";
                case StackTypeCode.Runtime:
                    return $"::natsu::ops::access({src.Expression}).";
                default:
                    throw new NotSupportedException();
            }
        }

        private static string MakeUnsignedExpression(StackEntry src)
        {
            // Try parse literal
            var exp = src.Expression;
            if (uint.TryParse(exp, out var u4))
            {
                return $"{u4}U";
            }
            else if (ulong.TryParse(exp, out var u8))
            {
                return $"{u8}ULL";
            }
            else
            {
                if (src.Type.Code == StackTypeCode.O)
                    return src.Expression;

                switch (src.Type.TypeSig.ElementType)
                {
                    case ElementType.U:
                    case ElementType.U1:
                    case ElementType.U2:
                    case ElementType.U4:
                    case ElementType.U8:
                    case ElementType.Class:
                    case ElementType.Object:
                    case ElementType.String:
                    case ElementType.Array:
                    case ElementType.SZArray:
                    case ElementType.Boolean:
                    case ElementType.Ptr:
                        return src.Expression;
                    default:
                        return $"::natsu::ops::unsign({src.Expression})";
                }
            }
        }

        public void Nop()
        {
            // Writer.Ident(Ident).WriteLine("::natsu::nop();");
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

        public void Endfinally()
        {
            Writer.Ident(Ident).WriteLine("return;");
        }

        public void Throw()
        {
            var v1 = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"::natsu::ops::throw_({v1.Expression});");
        }

        public void Ldloc()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            Stack.Push(TypeUtils.GetStackType(local.Type), $"{TypeUtils.GetLocalName(local, Method)}", computed: false);
        }

        public void Ldloca()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            Stack.Push(TypeUtils.GetStackType(new ByRefSig(local.Type)), $"::natsu::ops::ref({TypeUtils.GetLocalName(local, Method)})", computed: true);
        }

        public void Stloc()
        {
            var local = Op.GetLocal(Method.Body.Variables.ToList());
            var value = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"{TypeUtils.GetLocalName(local, Method)} = {CastExpression(local.Type, value)};");
        }

        public void Br()
        {
            var nextOp = (Instruction)Op.Operand;
            BranchUnconditional(Ident, nextOp);
        }

        public void Leave()
        {
            var nextOp = (Instruction)Op.Operand;
            BranchUnconditional(Ident, nextOp);
        }

        public void Unary(string op)
        {
            var v1 = Stack.Pop();
            Stack.Push(v1.Type, $"{op}{v1.Expression}");
        }

        public void Binary(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var type = TypeUtils.IsRefOrPtr(v1.Type) && TypeUtils.IsRefOrPtr(v2.Type)
                ? TypeUtils.GetStackType(CorLibTypes.IntPtr)
                : v1.Type;

            if (op == "%" && v1.Type.Code == StackTypeCode.F)
            {
                if (v1.Type.TypeSig.ElementType == ElementType.R4)
                    Stack.Push(type, $"fmodf({v1.Expression}, {v2.Expression})");
                else
                    Stack.Push(type, $"fmod({v1.Expression}, {v2.Expression})");
            }
            else
            {
                if (v1.Type.TypeSig.ElementType == ElementType.ValueType)
                    Stack.Push(v2.Type, $"({v1.Expression} {op} {v2.Expression})");
                else
                    Stack.Push(type, $"({v1.Expression} {op} {v2.Expression})");
            }
        }

        public void Binary_Ovf(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var type = TypeUtils.IsRefOrPtr(v1.Type) && TypeUtils.IsRefOrPtr(v2.Type)
                ? TypeUtils.GetStackType(CorLibTypes.IntPtr)
                : v1.Type;

            if (op == "%" && v1.Type.Code == StackTypeCode.F)
            {
                if (v1.Type.TypeSig.ElementType == ElementType.R4)
                    Stack.Push(type, $"fmodf({v1.Expression}, {v2.Expression})");
                else
                    Stack.Push(type, $"fmod({v1.Expression}, {v2.Expression})");
            }
            else
            {
                if (v1.Type.TypeSig.ElementType == ElementType.ValueType)
                    Stack.Push(v2.Type, $"({v1.Expression} {op} {v2.Expression})");
                else
                    Stack.Push(type, $"({v1.Expression} {op} {v2.Expression})");
            }
        }

        public void Binary_Ovf_Un(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var type = TypeUtils.IsRefOrPtr(v1.Type) && TypeUtils.IsRefOrPtr(v2.Type)
                ? TypeUtils.GetStackType(CorLibTypes.IntPtr)
                : v1.Type;

            if (op == "%" && v1.Type.Code == StackTypeCode.F)
            {
                if (v1.Type.TypeSig.ElementType == ElementType.R4)
                    Stack.Push(type, $"fmodf({v1.Expression}, {v2.Expression})");
                else
                    Stack.Push(type, $"fmod({v1.Expression}, {v2.Expression})");
            }
            else
            {
                if (v1.Type.TypeSig.ElementType == ElementType.ValueType)
                    Stack.Push(v2.Type, $"({v1.Expression} {op} {v2.Expression})");
                else
                    Stack.Push(type, $"({v1.Expression} {op} {v2.Expression})");
            }
        }

        public void Binary_Un(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var type = TypeUtils.IsRefOrPtr(v1.Type) && TypeUtils.IsRefOrPtr(v2.Type)
                ? TypeUtils.GetStackType(CorLibTypes.UIntPtr)
                : v1.Type;
            if (op == "%" && v1.Type.Code == StackTypeCode.F)
            {
                if (v1.Type.TypeSig.ElementType == ElementType.R4)
                    Stack.Push(type, $"fmodf({v1.Expression}, {v2.Expression})");
                else
                    Stack.Push(type, $"fmod({v1.Expression}, {v2.Expression})");
            }
            else
            {
                Stack.Push(type, $"({MakeUnsignedExpression(v1)} {op} {MakeUnsignedExpression(v2)})");
            }
        }

        public void Compare(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            if (op == ">" && v2.Type.Name == "std::nullptr_t")
                Stack.Push(CorLibTypes.Boolean, $"bool({v1.Expression})");
            else
                Stack.Push(CorLibTypes.Boolean, $"({v1.Expression} {op} {v2.Expression})");
        }

        public void Compare_Un(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            if (op == ">" && v2.Type.Name == "std::nullptr_t")
                Stack.Push(CorLibTypes.Boolean, $"bool({v1.Expression})");
            else
                Stack.Push(CorLibTypes.Boolean, $"({MakeUnsignedExpression(v1)} {op} {MakeUnsignedExpression(v2)})");
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
            Writer.Ident(Ident).WriteLine($"if ({v1.Expression} {op} {v2.Expression})");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetLabel(Method, nextOp, Block)};");
            Writer.Ident(Ident).WriteLine("else");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
        }

        private void BranchCompare_Un(string op)
        {
            var v2 = Stack.Pop();
            var v1 = Stack.Pop();
            var nextOp = (Instruction)Op.Operand;
            Writer.Ident(Ident).WriteLine($"if ({MakeUnsignedExpression(v1)} {op} {MakeUnsignedExpression(v2)})");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetLabel(Method, nextOp, Block)};");
            Writer.Ident(Ident).WriteLine("else");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
        }

        private void BranchIf(string op)
        {
            var v1 = Stack.Pop();
            var nextOp = (Instruction)Op.Operand;
            Writer.Ident(Ident).WriteLine($"if ({op}{v1.Expression})");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetLabel(Method, nextOp, Block)};");
            Writer.Ident(Ident).WriteLine("else");
            Writer.Ident(Ident + 1).WriteLine($"goto {ILUtils.GetFallthroughLabel(Method, Op, Block)};");
        }

        public void Ldstr()
        {
            var value = (string)Op.Operand;
            Stack.Push(CorLibTypes.String, $"::{ ModuleName}::user_string_{UserStrings.Count}.get()");
            UserStrings.Add(value);
        }

        public void Ldsfld()
        {
            var field = (IField)Op.Operand;
            string expr = Method.IsStaticConstructor && TypeUtils.IsSameType(Method.DeclaringType, field.DeclaringType)
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::get()." + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;
            if (Stack.PopVolatile())
                expr += ".load()";

            Stack.Push(TypeUtils.GetStackType(fieldType), $"{expr}");
        }

        public void Ldfld()
        {
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            string expr = MakeAccessExpression(target) + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            if (Stack.PopVolatile())
                expr += ".load()";
            Stack.Push(TypeUtils.GetStackType(fieldType), $"{expr}");
        }

        public void Ldflda()
        {
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            var fieldType = field.FieldSig.Type;
            if (TypeUtils.IsCppBasicType(field.DeclaringType))
            {
                if (target.Expression == "_this")
                    Stack.Push(TypeUtils.GetStackType(new ByRefSig(fieldType)), $"_this", computed: true);
                else
                    Stack.Push(TypeUtils.GetStackType(new ByRefSig(fieldType)), $"::natsu::ops::ref({target.Expression})", computed: true);
            }
            else
            {
                string expr = MakeAccessExpression(target) + TypeUtils.EscapeIdentifier(field.Name);
                Stack.Push(TypeUtils.GetStackType(new ByRefSig(fieldType)), $"::natsu::ops::ref({expr})", computed: true);
            }
        }

        public void Ldsflda()
        {
            var field = (IField)Op.Operand;
            var fieldType = field.FieldSig.Type;
            string expr = Method.IsStaticConstructor && TypeUtils.IsSameType(Method.DeclaringType, field.DeclaringType)
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::get()." + TypeUtils.EscapeIdentifier(field.Name);
            Stack.Push(TypeUtils.GetStackType(new ByRefSig(fieldType)), $"::natsu::ops::ref({expr})", computed: true);
        }

        public void Stsfld()
        {
            if (Method.IsStaticConstructor && Method.DeclaringType.Name.Contains("List"))
                ;
            var value = Stack.Pop();
            var field = (IField)Op.Operand;
            if (Method.IsStaticConstructor && Method.DeclaringType.FullName.Contains("List"))
                ;
            string expr = Method.IsStaticConstructor && TypeUtils.IsSameType(Method.DeclaringType, field.DeclaringType)
                ? TypeUtils.EscapeIdentifier(field.Name)
                : "::natsu::static_holder<typename" + TypeUtils.EscapeTypeName(field.DeclaringType) + "::Static>::get()." + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            if (Stack.PopVolatile())
                Writer.Ident(Ident).WriteLine($"{expr}.store({CastExpression(fieldType, value)});");
            else
                Writer.Ident(Ident).WriteLine($"{expr} = {CastExpression(fieldType, value)};");
        }

        public void Stfld()
        {
            var value = Stack.Pop();
            var target = Stack.Pop();
            var field = (IField)Op.Operand;
            var thisType = TypeUtils.ThisType(field.DeclaringType);
            string expr = MakeAccessExpression(target) + TypeUtils.EscapeIdentifier(field.Name);
            var fieldType = field.FieldSig.Type;

            if (Stack.PopVolatile())
                Writer.Ident(Ident).WriteLine($"{expr}.store({CastExpression(fieldType, value)});");
            else
                Writer.Ident(Ident).WriteLine($"{expr} = {CastExpression(fieldType, value)};");
        }

        public void Newarr()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var len = Stack.Pop();
            Stack.Push(new SZArraySig(type.ToTypeSig()).ToTypeDefOrRef(), $"::natsu::gc_new_array<{TypeUtils.EscapeTypeName(type, cppBasicType: true)}>({len.Expression})");
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
            var expr = $"::natsu::ops::newobj<{TypeUtils.EscapeTypeName(member.DeclaringType, cppBasicType: true)}>({string.Join(", ", para.Select(x => CastExpression(x.destType, x.src, genSig?.GenericArguments)))})";
            Stack.Push(TypeUtils.GetStackType(member.DeclaringType.ToTypeSig()), expr);
        }

        public void Ldobj()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var addr = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(type.ToTypeSig()), $"*{addr.Expression}");
        }

        public void Stobj()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var src = Stack.Pop();
            var dest = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"*{dest.Expression} = {src.Expression};");
        }

        public void Ldtoken()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            Stack.Push(CorLibTypes.Object, $"::natsu::ops::ldtoken_type<{TypeUtils.EscapeTypeName(type)}>()");
        }

        public void Isinst()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(StackTypeCode.O, type.ToTypeSig()), $"::natsu::ops::isinst<{TypeUtils.EscapeTypeName(type, cppBasicType: true)}>({obj.Expression})");
        }

        public void Unbox_Any()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(type.ToTypeSig()), $"::natsu::ops::unbox_any<{TypeUtils.EscapeTypeName(type, cppBasicType: true)}>({obj.Expression})");
        }

        public void Unbox()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(new ByRefSig(type.ToTypeSig())), $"::natsu::ops::unbox<{TypeUtils.EscapeTypeName(type, cppBasicType: true)}>({obj.Expression})");
        }

        public void Castclass()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var obj = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(type.ToTypeSig()), $"::natsu::ops::castclass<{TypeUtils.EscapeTypeName(type, cppBasicType: true)}>({obj.Expression})");
        }

        public void Ldlen()
        {
            var target = Stack.Pop();
            Stack.Push(CorLibTypes.UIntPtr, $"{target.Expression}->length()");
        }

        public void Localloc()
        {
            var size = Stack.Pop();
            Stack.Push(CorLibTypes.UIntPtr, $"natsu_alloca({size.Expression})");
        }

        private void Conversion(TypeSig stackType, string type)
        {
            var value = Stack.Pop();
            var expr = value.Expression;
            if (TypeUtils.IsRefOrPtr(value.Type))
                expr += ".ptr_";

            string destTypeName = null;
            if (stackType.ElementType == ElementType.I ||
                stackType.ElementType == ElementType.U)
            {
                if (TypeUtils.IsRefOrPtr(value.Type))
                {
                    destTypeName = stackType.ElementType switch
                    {
                        ElementType.I => "reinterpret_cast<intptr_t>",
                        ElementType.U => "reinterpret_cast<uintptr_t>",
                    };
                }
                else
                {
                    destTypeName = stackType.ElementType switch
                    {
                        ElementType.I => "static_cast<intptr_t>",
                        ElementType.U => "static_cast<uintptr_t>",
                    };
                }
            }
            else
            {
                switch (stackType.ElementType)
                {
                    case ElementType.I1:
                    case ElementType.I2:
                    case ElementType.I4:
                    case ElementType.I8:
                    case ElementType.U1:
                    case ElementType.U2:
                    case ElementType.U4:
                    case ElementType.U8:
                        if (TypeUtils.IsRefOrPtr(value.Type))
                            destTypeName = $"reinterpret_cast<{TypeUtils.EscapeVariableTypeName(stackType)}>";
                        break;
                }
            }

            destTypeName = destTypeName ?? $"static_cast<{TypeUtils.EscapeVariableTypeName(stackType)}>";
            Stack.Push(stackType, $"{destTypeName}({expr})");
        }

        private void Ldind(TypeSig stackType)
        {
            var addr = Stack.Pop();
            if (addr.Type.TypeSig.ElementType == ElementType.ByRef ||
                addr.Type.TypeSig.ElementType == ElementType.Ptr)
            {
                var actualType = addr.Type.TypeSig.Next;
                if (TypeUtils.IsSameType(actualType.ToTypeDefOrRef(), stackType.ToTypeDefOrRef()))
                {
                    Stack.Push(stackType, $"*{addr.Expression}");
                    return;
                }
            }

            Stack.Push(stackType, $"::natsu::ops::ldind<{TypeUtils.EscapeVariableTypeName(stackType)}>({addr.Expression})");
        }

        private void Stind(TypeSig stackType)
        {
            var value = Stack.Pop();
            var addr = Stack.Pop();

            if (addr.Expression == "str" && value.Expression == "p")
                ;

            if (addr.Type.TypeSig.ElementType == ElementType.ByRef ||
                addr.Type.TypeSig.ElementType == ElementType.Ptr)
            {
                var actualType = addr.Type.TypeSig.Next;
                if (TypeUtils.IsSameType(actualType.ToTypeDefOrRef(), stackType.ToTypeDefOrRef())
                    || TypeUtils.IsSameType(actualType.ToTypeDefOrRef(), value.Type.TypeSig.ToTypeDefOrRef()))
                {
                    Writer.Ident(Ident).WriteLine($"*{addr.Expression} = {value.Expression};");
                    return;
                }
            }

            Writer.Ident(Ident).WriteLine($"::natsu::ops::stind<{TypeUtils.EscapeVariableTypeName(stackType)}>({addr.Expression}, {value.Expression});");
        }

        private void Ldelem(TypeSig stackType, string type)
        {
            var index = Stack.Pop();
            var array = Stack.Pop();
            Stack.Push(stackType, $"{array.Expression}->at({index.Expression})");
        }

        private void Stelem(string type)
        {
            var value = Stack.Pop();
            var index = Stack.Pop();
            var array = Stack.Pop();
            //Writer.Ident(Ident).WriteLine($"::natsu::ops::stelem_{type}({array.Expression}, {index.Expression}, {value.Expression});");
            Writer.Ident(Ident).WriteLine($"{array.Expression}->at({index.Expression}) = {value.Expression};");
        }

        public void Ldelem()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var index = Stack.Pop();
            var array = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(type.ToTypeSig()), $"{array.Expression}->at({index.Expression})");
        }

        public void Stelem()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var value = Stack.Pop();
            var index = Stack.Pop();
            var array = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"{array.Expression}->at({index.Expression}) = {value.Expression};");
        }

        public void Ldelema()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var index = Stack.Pop();
            var array = Stack.Pop();
            Stack.Push(TypeUtils.GetStackType(new ByRefSig(type.ToTypeSig())), $"{array.Expression}->ref_at({index.Expression})");
        }

        public void Box()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            var value = Stack.Pop();
            Stack.Push(CorLibTypes.Object, $"::natsu::ops::box({CastExpression(type.ToTypeSig(), value)})");
        }

        public void Ldnull()
        {
            Stack.Push(new StackType { Code = StackTypeCode.O, TypeSig = CorLibTypes.Object, Name = "std::nullptr_t" }, "nullptr");
        }

        public void Dup()
        {
            Stack.Dup();
        }

        public void Ldftn()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var tGen = (member.DeclaringType as TypeSpec)?.TryGetGenericInstSig()?.GenericArguments.ToList()
                ?? new List<TypeSig>();
            var gen = (member as MethodSpec)?.GenericInstMethodSig;

            string expr;
            if (gen != null)
            {
                var genArgs = gen.GenericArguments;
                tGen.AddRange(genArgs);
                expr = $"{TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}<{string.Join(", ", gen.GenericArguments.Select(x => TypeUtils.EscapeTypeName(x)))}>";
            }
            else
            {
                expr = $"{ TypeUtils.EscapeTypeName(member.DeclaringType)}::{TypeUtils.EscapeMethodName(member, hasParamType: false)}";
            }

            Stack.Push(CorLibTypes.IntPtr, $"::natsu::ops::ldftn({expr})");
        }

        public void Ldvirtftn()
        {
            var member = (IMethod)Op.Operand;
            var method = member.MethodSig;
            var tGen = (member.DeclaringType as TypeSpec)?.TryGetGenericInstSig()?.GenericArguments.ToList()
                ?? new List<TypeSig>();
            var gen = (member as MethodSpec)?.GenericInstMethodSig;

            string expr;
            if (gen != null)
            {
                throw new NotSupportedException();
            }
            else
            {
                var obj = Stack.Pop();
                expr = $"{obj.Expression}.header().template vtable_as<typename {TypeUtils.EscapeTypeName(member.DeclaringType)}::VTable>()->{TypeUtils.EscapeMethodName(member)}";
            }

            Stack.Push(CorLibTypes.IntPtr, $"::natsu::ops::ldftn({expr})");
        }

        public void Switch()
        {
            var instructions = (Instruction[])Op.Operand;
            var v1 = Stack.Pop();
            Writer.Ident(Ident).WriteLine($"switch ({v1.Expression})");
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

        public void Sizeof()
        {
            var type = (ITypeDefOrRef)Op.Operand;
            Stack.Push(CorLibTypes.Int32, $"sizeof({TypeUtils.EscapeTypeName(type)})");
        }

        public void Pop()
        {
            Stack.Compute();
            Stack.Pop();
        }
    }
}
