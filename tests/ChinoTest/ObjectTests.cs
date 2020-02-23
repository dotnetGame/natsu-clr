using System;
using System.Collections.Generic;
using System.Text;
using Chino.IO;
using Chino.IO.Devices;
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

        private class TestDevice : ConsoleDevice
        {

        }

        private class TestDriver : Driver
        {
            protected override void InstallDevice(DeviceDescription deviceDescription)
            {
                IOManager.InstallDevice(new TestDevice());
            }

            protected override bool IsCompatible(DeviceDescription deviceDescription)
            {
                return true;
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

        [Fact]
        public void TestCreateDevice()
        {
            IOManager.InstallDriver("test.test", new TestDriver());
            IOManager.RegisterDeviceDescription(new DeviceDescription("test.console"));
            var device = ObjectManager.OpenObject<Device>(AccessMask.GenericRead, new ObjectAttributes { Name = "/dev/console0" });
        }
    }
}
