#include "System.Private.CoreLib.h"
#include <Windows.h>

using namespace natsu;
using namespace System_Private_CoreLib::System::Threading;

void Interlocked::_s__MemoryBarrierProcessWide()
{
    FlushProcessWriteBuffers();
}
