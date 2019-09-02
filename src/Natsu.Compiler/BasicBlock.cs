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
    }
}
