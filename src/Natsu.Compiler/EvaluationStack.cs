using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using dnlib.DotNet;

namespace Natsu.Compiler
{
    public enum StackTypeCode
    {
        Void,
        Int32,
        Int64,
        NativeInt,
        F,
        O,
        Ref,
        Runtime,
        ValueType
    }

    public struct StackType
    {
        public StackTypeCode Code;

        public TypeSig TypeSig;

        public IList<TypeSig> GenArgs;

        public string Name;
    }

    class StackEntry
    {
        public StackType Type { get; set; }

        public string Expression { get; set; }

        public bool Computed { get; set; }

        public override string ToString()
        {
            return $"{Type}, {Expression}";
        }
    }

    class EvaluationStack
    {
        private readonly Stack<StackEntry> _stackValues = new Stack<StackEntry>();
        private int _paramIndex = 0;
        private TextWriter _writer;
        public int Ident { get; set; }
        public int ParamIndex => _paramIndex;

        public ITypeDefOrRef Constrained { get; set; }

        public bool Empty => _stackValues.Count == 0;

        public int Count => _stackValues.Count;

        public bool Volatile { get; set; }

        public bool PopVolatile()
        {
            var value = Volatile;
            Volatile = false;
            return value;
        }

        public EvaluationStack(TextWriter writer, int ident, int paramIndex)
        {
            _writer = writer;
            Ident = ident;
            _paramIndex = paramIndex;
        }

        public void SetWriter(TextWriter writer)
        {
            _writer = writer;
        }

        public void Push(StackEntry entry)
        {
            if (entry.Type.Code == StackTypeCode.Void)
            {
                _writer.Ident(Ident).WriteLine($"{entry.Expression};");
            }
            else
            {
                //var id = $"_v{_paramIndex++}";
                //_writer.Ident(Ident).WriteLine($"auto {id} = {entry.Expression};");
                //_stackValues.Push(new StackEntry { Type = entry.Type, Expression = id });
                _stackValues.Push(entry);
            }
        }

        public void Compute()
        {
            var entry = _stackValues.Peek();
            if (!entry.Computed)
            {
                _stackValues.Pop();
                var id = $"_v{_paramIndex++}";
                _writer.Ident(Ident).WriteLine($"auto {id} = {entry.Expression};");
                var newEntry = new StackEntry { Type = entry.Type, Expression = id, Computed = true };
                _stackValues.Push(newEntry);
            }
        }

        public void Dup()
        {
            Compute();
            Push(Peek());
        }

        public void Push(StackType type, string expression, bool computed = false)
        {
            Push(new StackEntry { Type = type, Expression = expression, Computed = computed });
        }

        public void Push(ITypeDefOrRef type, string expression, bool computed = false)
        {
            Push(new StackEntry { Type = TypeUtils.GetStackType(type.ToTypeSig()), Expression = expression, Computed = computed });
        }

        public void Push(TypeSig type, string expression, bool computed = false)
        {
            Push(new StackEntry { Type = TypeUtils.GetStackType(type), Expression = expression, Computed = computed });
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
            var stack = new EvaluationStack(_writer, Ident + identInc, _paramIndex);
            foreach (var value in _stackValues.Reverse())
                stack._stackValues.Push(value);
            return stack;
        }
    }

    class SpillSlot
    {
        public int Index { get; set; }

        public string Name { get; set; }

        public StackEntry Entry { get; set; }

        public List<BasicBlock> Next { get; set; }
    }
}
