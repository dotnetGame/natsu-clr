#include "Generated/Chino.Kernel.h"
#include <iostream>
#include <Windows.h>

using namespace Chino_Kernel::Chino::Kernel;

void InitializeHeap() noexcept;

int main()
{
    InitializeHeap();
    Program::_s_Main();
    std::cout << sizeof(System_Private_CorLib::System::Boolean) << std::endl;
    return 0;
}
