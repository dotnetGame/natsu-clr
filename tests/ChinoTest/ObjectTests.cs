using System;
using System.Collections.Generic;
using System.Text;
using Chino.Objects;
using Xunit;

namespace ChinoTest
{
    public class ObjectTests
    {
        private class TestObject : Chino.Objects.Object
        {
            public override bool CanOpen => true;

            protected override void Open(AccessMask grantedAccess)
            {
            }
        }

        [Fact]
        public void TestCreateDirectory()
        {
            ObjectManager.CreateDirectory(AccessMask.GenericAll, new ObjectAttributes { Name = "dev" });
            var device = ObjectManager.OpenDirectory(AccessMask.GenericAll, new ObjectAttributes { Name = "/dev" });

            Assert.Equal("dev", device.Object.Name);
        }

        [Fact]
        public void TestCreateObject()
        {
            ObjectManager.CreateObject(new TestObject(), AccessMask.GenericAll, new ObjectAttributes { Name = "obj" });
            var obj = ObjectManager.OpenObject<TestObject>(AccessMask.GenericAll, new ObjectAttributes { Name = "/obj" });

            Assert.Equal("obj", obj.Object.Name);
        }
    }
}
