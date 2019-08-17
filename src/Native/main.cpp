#include "Generated/Chino.Kernel.h"

using namespace Chino_Kernel::Chino::Kernel;

void InitializeHeap() noexcept;

int main()
{
    InitializeHeap();
    Program::_s_Main();
    return 0;
}
