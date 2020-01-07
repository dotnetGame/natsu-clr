using Chino.Collections;
using System;
using System.Collections.Generic;
using System.Text;
using Xunit;

namespace ChinoTest
{
    public class ValueRingBufferTests
    {
        [Fact]
        public void TestSimpleWriteRead()
        {
            var b = new ValueRingBuffer<int>(16);
            Assert.Equal(3, b.TryWrite(stackalloc int[] { 1, 2, 3 }));
            Span<int> tmp = stackalloc int[3];
            Assert.Equal(3, b.TryRead(tmp));
            Assert.Equal(new[] { 1, 2, 3 }, tmp.ToArray());
        }
    }
}
