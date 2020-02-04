using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Text;

namespace Chino.Objects
{
    public static class ObjectManager
    {
        public static readonly char DirectorySeparatorChar = '/';

        private static Directory Root { get; }

        static ObjectManager()
        {
            Root = new Directory();
        }

        public static Accessor<Directory> CreateDirectory(AccessMask desiredAccess, in ObjectAttributes attributes)
        {
            var dir = new Directory();
            AttachDirectoryEntry(dir, attributes);
            return new Accessor<Directory>(dir, desiredAccess);
        }

        public static Accessor<Directory> OpenDirectory(AccessMask desiredAccess, in ObjectAttributes attributes)
        {
            var accessState = new AccessState(desiredAccess);
            return OpenObject<Directory>(attributes, ref accessState);
        }

        public static Accessor<T> CreateObject<T>(T @object, AccessMask desiredAccess, in ObjectAttributes attributes)
            where T : Object
        {
            if (!string.IsNullOrEmpty(attributes.Name))
                AttachDirectoryEntry(@object, attributes);
            return new Accessor<T>(@object, desiredAccess);
        }

        public static Accessor<T> OpenObject<T>(AccessMask desiredAccess, in ObjectAttributes attributes)
            where T : Object
        {
            var accessState = new AccessState(desiredAccess);
            return OpenObject<T>(attributes, ref accessState);
        }

        public static ObjectParseStatus TryOpenObject<T>(AccessMask desiredAccess, in ObjectAttributes attributes, out Accessor<T> accessor)
            where T : Object
        {
            var accessState = new AccessState(desiredAccess);
            return TryOpenObject(attributes, ref accessState, out accessor);
        }

        public static Accessor<T> OpenObject<T>(in Accessor<T> accessor, AccessMask desiredAccess)
            where T : Object
        {
            var accessState = new AccessState(desiredAccess);
            return OpenObject(accessor.Object, ref accessState);
        }

        public static Accessor<T> OpenObject<T>(T @object, AccessMask desiredAccess)
            where T : Object
        {
            var accessState = new AccessState(desiredAccess);
            return OpenObject(@object, ref accessState);
        }

        private static ObjectParseStatus TryFindObject(string? path, ref AccessState accessState, out Object? foundObject, Directory? parent = null)
        {
            if (parent != null)
            {
                // Start with '/' and can't parse, it is a bad path syntax
                if (!string.IsNullOrEmpty(path) && path[0] == DirectorySeparatorChar && !parent.CanParse)
                {
                    foundObject = null;
                    return ObjectParseStatus.InvalidPath;
                }
            }
            else if (string.IsNullOrEmpty(path))
            {
                foundObject = Root;
                return ObjectParseStatus.Success;
            }
            else
            {
                // From root, path should start with '/'
                if (string.IsNullOrEmpty(path) || path[0] != DirectorySeparatorChar)
                {
                    foundObject = null;
                    return ObjectParseStatus.InvalidPath;
                }

                parent = Root;
            }

            // Skip first '/'
            var completeName = path.AsSpan();
            var remainingName = completeName;
            if (!remainingName.IsEmpty && remainingName[0] == DirectorySeparatorChar)
                remainingName = remainingName.Slice(1);

            if (!parent.CanParse)
            {
                foundObject = null;
                return ObjectParseStatus.InvalidPath;
            }

            return parent.Parse(ref accessState, ref completeName, ref remainingName, out foundObject);
        }

        private static Object FindObject(string? path, ref AccessState accessState, Directory? parent = null)
        {
            var status = TryFindObject(path, ref accessState, out var foundObject, parent);
            switch (status)
            {
                case ObjectParseStatus.InvalidPath:
                    throw new ArgumentException(nameof(path));
                case ObjectParseStatus.NotFound:
                    throw new FileNotFoundException();
            }

            Debug.Assert(foundObject != null);
            return foundObject;
        }

        private static void AttachDirectoryEntry(Object @object, in ObjectAttributes attributes)
        {
            Debug.Assert(@object._header.Parent == null);
            ValidateObjectName(attributes.Name);
            var parent = attributes.Root?.Object ?? Root;
            @object._header.Name = attributes.Name;
            @object._header.Parent = parent;
            parent.Add(@object);
        }

        private static Accessor<T> OpenObject<T>(in ObjectAttributes attributes, ref AccessState accessState)
            where T : Object
        {
            var obj = (T)FindObject(attributes.Name, ref accessState, attributes.Root?.Object);
            return OpenObject(obj, ref accessState);
        }

        private static ObjectParseStatus TryOpenObject<T>(in ObjectAttributes attributes, ref AccessState accessState, out Accessor<T> accessor)
            where T : Object
        {
            var status = TryFindObject(attributes.Name, ref accessState, out var @object, attributes.Root?.Object);
            if (status == ObjectParseStatus.Success)
            {
                if (@object is T obj)
                {
                    accessor = OpenObject<T>(obj, ref accessState);
                    return ObjectParseStatus.Success;
                }
                else
                {
                    accessor = default;
                    return ObjectParseStatus.NotFound;
                }
            }

            accessor = default;
            return status;
        }

        private static Accessor<T> OpenObject<T>(T @object, ref AccessState accessState)
            where T : Object
        {
            @object.Open(accessState.PreviouslyGrantedAccess);
            var desiredAccess = accessState.RemainingDesiredAccess | accessState.PreviouslyGrantedAccess;
            var grantedAccess = desiredAccess & @object.ValidAccessMask;
            accessState.PreviouslyGrantedAccess = grantedAccess;
            return new Accessor<T>(@object, grantedAccess);
        }

        public static void ValidateObjectName(string? name)
        {
            Debug.Assert(name != null);
            Debug.Assert(!name.Contains(DirectorySeparatorChar));
        }
    }
}
