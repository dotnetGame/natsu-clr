using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using dnlib.DotNet;
using dnlib.DotNet.Emit;

namespace Natsu.Compiler
{
    static class ILUtils
    {
        public static string GetLabel(MethodDef method, int id)
        {
            return $"M{method.Rid:X4}_{id}";
        }

        public static string GetLabel(MethodDef method, Instruction instruction, BasicBlock block)
        {
            var id = instruction == block.Instructions[0]
                ? block.Id
                : block.Next.First(x => x.Instructions[0] == instruction).Id;
            return GetLabel(method, id);
        }

        public static string GetFallthroughLabel(MethodDef method, Instruction instruction, BasicBlock block)
        {
            var nextInst = method.Body.Instructions[method.Body.Instructions.IndexOf(instruction) + 1];
            var id = block.Next.First(x => x.Instructions[0] == nextInst).Id;
            return GetLabel(method, id);
        }
    }
}
