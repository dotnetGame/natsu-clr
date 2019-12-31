// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Reflection;
using System.Runtime.Serialization;
using System.Diagnostics;
using Internal.Runtime.CompilerServices;
using System.Runtime.CompilerServices;

namespace System
{
    public abstract class MulticastDelegate : Delegate
    {
        // This is set under 3 circumstances
        // 1. Multicast delegate
        // 2. Secure/Wrapper delegate
        // 3. Inner delegate of secure delegate where the secure delegate security context is a collectible method
        private Delegate[]? _invocationList;

        protected override Delegate CombineImpl(Delegate newDelegate)
        {
            var b = (MulticastDelegate)newDelegate;
            var bList = b._invocationList;
            var aList = _invocationList;
            var aNum = aList == null ? 1 : aList.Length;
            var bNum = bList == null ? 1 : bList.Length;
            var newList = new Delegate[aNum + bNum];
            CopyDelegate(aNum, this, aList, newList, 0);
            CopyDelegate(bNum, b, bList, newList, aNum);
            return CreateDelegateLike(this, newList);
        }

        protected override Delegate? RemoveImpl(Delegate d)
        {
            var b = (MulticastDelegate)d;
            var bList = b._invocationList;
            var aList = _invocationList;
            var aNum = aList == null ? 1 : aList.Length;
            var bNum = bList == null ? 1 : bList.Length;

            if (aNum == 1)
            {
                if (bNum == 1)
                {
                    if (ReferenceEquals(this, b))
                        return null;
                }

                return this;
            }
            else
            {
                if (bNum == 1)
                {
                    for (int i = aNum - 1; i >= 0; i--)
                    {
                        if (aList[i] == d)
                        {
                            var newList = new Delegate[aNum - 1];
                            Array.Copy(aList, 0, newList, 0, i);
                            if (i != aNum - 1)
                                Array.Copy(aList, i + 1, newList, i, aNum - i);
                            return CreateDelegateLike(this, newList);
                        }
                    }
                }
                else
                {
                    for (int i = aNum - bNum; i >= 0; i--)
                    {
                        bool equals = true;
                        for (int j = 0; j < bNum; j++)
                        {
                            if (aList[i + j] != bList[j])
                            {
                                equals = false;
                                break;
                            }
                        }

                        if (equals)
                        {
                            var newList = new Delegate[aNum - bNum];
                            Array.Copy(aList, 0, newList, 0, i);
                            if (i != aNum - bNum)
                                Array.Copy(aList, i + bNum, newList, i, aNum - bNum);
                            return CreateDelegateLike(this, newList);
                        }
                    }
                }
            }

            return this;
        }

        private void CopyDelegate(int num, Delegate src, Delegate[] srcList, Delegate[] destList, int offset)
        {
            if (num == 1)
                destList[offset] = src;
            else
                Array.Copy(srcList, destList, offset);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern MulticastDelegate CreateDelegateLike(MulticastDelegate @delegate, Delegate[] invocationList);
    }
}
