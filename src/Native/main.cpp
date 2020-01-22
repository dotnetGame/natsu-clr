#include "Generated/Chino.Kernel.h"
#include <algorithm>
#ifdef WIN32
#include <Windows.h>
#endif

using namespace Chino_Kernel::Chino::Kernel;

void InitializeHeap() noexcept;

int main()
{
    int32_t followCount = 0, resultCount = 0;
#line 536 "D:\\Work\\Repository\\corert.git\\src\\System.Private.CoreLib\\src\\System\\Delegate.cs"
    int32_t _35 = followCount; int32_t _36 = 1 + 1; resultCount = (int32_t)_36;

    InitializeHeap();
    Program::_s_Main();

    // Should not reach here
    while (1)
        ;
    return 0;
}
