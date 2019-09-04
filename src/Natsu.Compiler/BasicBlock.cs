using System;
using System.Collections.Generic;
using System.Text;
using dnlib.DotNet.Emit;

namespace Natsu.Compiler
{
    class BasicBlock
    {
        public int Id { get; set; }

        public List<Instruction> Instructions { get; } = new List<Instruction>();

        public BasicBlock Parent { get; set; }

        public List<BasicBlock> Next { get; set; } = new List<BasicBlock>();

        public bool Contains(Instruction instruction)
        {
            if (Instructions.Count != 0 && Instructions[0] == instruction)
                return true;
            if (Parent != null)
                return Parent.Contains(instruction);
            return false;
        }
    }
}
