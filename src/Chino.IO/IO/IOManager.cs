using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Chino.Objects;

namespace Chino.IO
{
    public static class IOManager
    {
        private static readonly Accessor<Directory> _driverDirectory;
        private static readonly Accessor<Directory> _deviceDirectory;
        private static readonly List<Accessor<Driver>> _drivers = new List<Accessor<Driver>>();
        private static readonly Dictionary<DeviceType, List<Accessor<Device>>> _devices = new Dictionary<DeviceType, List<Accessor<Device>>>();
        private static readonly List<DeviceDescription> _deviceDescriptions = new List<DeviceDescription>();

        static IOManager()
        {
            _driverDirectory = ObjectManager.CreateDirectory(
                AccessMask.GenericAll, new ObjectAttributes { Name = "drivers" });
            _deviceDirectory = ObjectManager.CreateDirectory(
                AccessMask.GenericAll, new ObjectAttributes { Name = "dev" });
        }

        public static void RegisterDeviceDescription(DeviceDescription deviceDescription)
        {
            lock (_deviceDescriptions)
                _deviceDescriptions.Add(deviceDescription);
            InstallCompatibleDriver(deviceDescription);
        }

        public static Accessor<Driver> InstallDriver(string name, Driver driver)
        {
            var accessor = ObjectManager.CreateObject(driver, AccessMask.GenericAll, new ObjectAttributes
            {
                Name = name,
                Root = _driverDirectory
            });
            lock (_drivers)
                _drivers.Add(accessor);
            driver.OnInstall();
            return accessor;
        }

        public static Accessor<Device> InstallDevice(Device device)
        {
            var accessors = GetDeviceStorage(device.DeviceType);
            Accessor<Device> accessor;
            lock (accessors)
            {
                var name = GetDeviceNamePrefix(device.DeviceType) + accessors.Count.ToString();
                accessor = ObjectManager.CreateObject(device, AccessMask.GenericAll, new ObjectAttributes
                {
                    Name = name,
                    Root = _deviceDirectory
                });
            }

            accessors.Add(accessor);
            accessor.Object.OnInstall();
            return accessor;
        }

        private static List<Accessor<Device>> GetDeviceStorage(DeviceType deviceType)
        {
            lock (_devices)
            {
                if (!_devices.TryGetValue(deviceType, out var accessors))
                {
                    accessors = new List<Accessor<Device>>();
                    _devices.Add(deviceType, accessors);
                }

                return accessors;
            }
        }

        private static bool InstallCompatibleDriver(DeviceDescription deviceDescription)
        {
            Debug.Assert(deviceDescription._installedDriver == null);

            Accessor<Driver>? foundDriver = null;
            if (!string.IsNullOrEmpty(deviceDescription.DesiredDriver))
            {
                if (ObjectManager.TryOpenObject<Driver>(AccessMask.GenericAll, new ObjectAttributes
                {
                    Name = deviceDescription.DesiredDriver,
                    Root = _driverDirectory
                }, out var driver) == ObjectParseStatus.Success)
                {
                    if (driver.Object.IsCompatible(deviceDescription))
                        foundDriver = driver;
                }
            }

            if (foundDriver == null)
            {
                lock (_drivers)
                {
                    foreach (var driver in _drivers)
                    {
                        if (driver.Object.IsCompatible(deviceDescription))
                        {
                            foundDriver = driver;
                            break;
                        }
                    }
                }
            }

            if (foundDriver != null)
            {
                foundDriver.Object.InstallDevice(deviceDescription);
                deviceDescription._installedDriver = foundDriver;
                return true;
            }

            return false;
        }

        private static string GetDeviceNamePrefix(DeviceType deviceType)
        {
            return deviceType switch
            {
                DeviceType.Console => "console",
                _ => throw new ArgumentOutOfRangeException(nameof(deviceType))
            };
        }
    }
}
