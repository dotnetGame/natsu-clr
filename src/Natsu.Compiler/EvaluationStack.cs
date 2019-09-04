using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using dnlib.DotNet;

namespace Natsu.Compiler
{
    enum StackType
    {
        Void,
        Int32,
        Int64,
        NativeInt,
        F,
        O,
        Ref,
        ValueType,
        Var
    }

    class StackEntry
    {
        public StackType Type { get; set; }

        public string Expression { get; set; }

        public override string ToString()
        {
            return $"{Type}, {Expression}";
        }
    }

    class EvaluationStack
    {
        private readonly Stack<StackEntry> _stackValues = new Stack<StackEntry>();
        private int _paramIndex = 0;
        private StreamWriter _writer;
        public int Ident { get; }

        public ITypeDefOrRef Constrained { get; set; }

        public EvaluationStack(StreamWriter writer, int ident)
        {
            _writer = writer;
            Ident = ident;
        }

        public void Push(StackEntry entry)
        {
            if (entry.Type == StackType.Void)
            {
                _writer.Ident(Ident).WriteLine($"{entry.Expression};");
            }
            else
            {
                var id = $"_v{_paramIndex++}";
                _writer.Ident(Ident).WriteLine($"auto {id} = {entry.Expression};");
                _stackValues.Push(new StackEntry { Type = entry.Type, Expression = id });
            }
        }

        public void Push(StackType type, string expression)
        {
            Push(new StackEntry { Type = type, Expression = expression });
        }

        public StackEntry Pop()
        {
            return _stackValues.Pop();
        }

        public StackEntry Peek()
        {
            return _stackValues.Peek();
        }

        public EvaluationStack Clone(int identInc = 0)
        {
            var stack = new EvaluationStack(_writer, Ident + identInc);
            foreach (var value in _stackValues.Reverse())
                stack._stackValues.Push(value);
            stack._paramIndex = _paramIndex;
            return stack;
        }
    }
}
