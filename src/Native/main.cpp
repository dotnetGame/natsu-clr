#include "Generated/Chino.Kernel.h"
#include <algorithm>
#if WIN32
#include <Windows.h>
#endif

using namespace Chino_Kernel::Chino::Kernel;

void InitializeHeap() noexcept;

int main()
{
    InitializeHeap();
    Program::_s_Main();
#if WIN32
    while (1)
        Sleep(0);
#else
    while (1)
        ;
#endif
    return 0;
}
