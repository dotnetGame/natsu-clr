#include "Generated/Chino.Kernel.h"
#include <algorithm>
#ifdef WIN32
#include <Windows.h>
#endif

using namespace Chino_Kernel::Chino::Kernel;

void InitializeHeap() noexcept;

int main()
{
    InitializeHeap();
    Program::_s_Main();

    // Should not reach here
    while (1)
        ;
    return 0;
}
