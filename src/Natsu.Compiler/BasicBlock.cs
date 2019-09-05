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

        public List<BasicBlock> Next { get; set; } = new List<BasicBlock>();

        public BasicBlock Parent { get; set; }

        public string Text { get; set; }

        public List<SpillSlot> Spills { get; } = new List<SpillSlot>();
    }

    class BlockGraph
    {
        public Dictionary<Instruction, BasicBlock> Blocks { get; } = new Dictionary<Instruction, BasicBlock>();
    }
}
