// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.Serialization;

namespace System.IO
{
    // Thrown when trying to access a file that doesn't exist on disk.
    [Serializable]
    [System.Runtime.CompilerServices.TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public partial class FileNotFoundException : IOException
    {
        public FileNotFoundException()
            : base(SR.IO_FileNotFound)
        {
            HResult = HResults.COR_E_FILENOTFOUND;
        }

        public FileNotFoundException(string message)
            : base(message)
        {
            HResult = HResults.COR_E_FILENOTFOUND;
        }

        public FileNotFoundException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = HResults.COR_E_FILENOTFOUND;
        }

        public FileNotFoundException(string message, string fileName) 
            : base(message)
        {
            HResult = HResults.COR_E_FILENOTFOUND;
            FileName = fileName;
        }

        public FileNotFoundException(string message, string fileName, Exception innerException)
            : base(message, innerException)
        {
            HResult = HResults.COR_E_FILENOTFOUND;
            FileName = fileName;
        }

        public string FileName { get; }
        public string FusionLog { get; }
    }
}

