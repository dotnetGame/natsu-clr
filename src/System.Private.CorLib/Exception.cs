using System;
using System.Runtime.Serialization;

namespace System
{
    public class Exception
    {
        public int HResult { get; protected set; }

        public virtual string Message { get; }

        public Exception InnerException { get; }

        public Exception()
        {

        }

        public Exception(string message)
        {
            Message = message;
        }

        public Exception(string message, Exception innerException)
        {
            Message = message;
            InnerException = innerException;
        }
    }
}
