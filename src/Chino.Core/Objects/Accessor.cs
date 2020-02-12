using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Objects
{
    public interface IAccessor<out T> : IDisposable where T : Object
    {
        public T Object { get; }

        public AccessMask GrantedAccess { get; }
    }

    public sealed class Accessor<T> : IAccessor<T> where T : Object
    {
        private T _object;
        public T Object
        {
            get
            {
                if (_disposedValue)
                    throw new ObjectDisposedException(nameof(Object));
                return _object;
            }
        }

        public AccessMask GrantedAccess { get; }

        internal Accessor(T @object, AccessMask grantedAccess)
        {
            _object = @object;
            GrantedAccess = grantedAccess;
        }

        public Accessor<U> Cast<U>() where U : Object
        {
            return new Accessor<U>((U)(object)Object, GrantedAccess);
        }

        #region IDisposable Support
        private bool _disposedValue = false; // 要检测冗余调用

        void Dispose(bool disposing)
        {
            if (!_disposedValue)
            {
                if (disposing)
                {
                    // TODO: 释放托管状态(托管对象)。
                }

                // TODO: 释放未托管的资源(未托管的对象)并在以下内容中替代终结器。
                // TODO: 将大型字段设置为 null。

                _disposedValue = true;
            }
        }

        // TODO: 仅当以上 Dispose(bool disposing) 拥有用于释放未托管资源的代码时才替代终结器。
        // ~Accessor()
        // {
        //   // 请勿更改此代码。将清理代码放入以上 Dispose(bool disposing) 中。
        //   Dispose(false);
        // }

        // 添加此代码以正确实现可处置模式。
        public void Dispose()
        {
            // 请勿更改此代码。将清理代码放入以上 Dispose(bool disposing) 中。
            Dispose(true);
            // TODO: 如果在以上内容中替代了终结器，则取消注释以下行。
            // GC.SuppressFinalize(this);
        }
        #endregion
    }
}
