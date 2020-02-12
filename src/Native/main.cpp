#include "Generated/Chino.Chip.Emulator.h"
#include "Generated/Chino.Chip.K210.h"
#include "Generated/Chino.Kernel.h"
#include <algorithm>
#ifdef WIN32
#include <Windows.h>
#endif

#define CONCAT(a, b) a##b
#define CHINO_CHIP_MODULE(name) CONCAT(Chino_Chip_, name)

using namespace Chino_Core::Chino::Chip;
using namespace Chino_Kernel::Chino::Kernel;
using namespace CHINO_CHIP_MODULE(CHINO_ARCH)::Chino::Chip;
using namespace natsu;

void InitializeHeap() noexcept;

int main()
{
    InitializeHeap();
    // Initialize chip control
    ChipControl::_s_set_Default(make_object<ArchChipControl>());
    KernelProgram::_s_KernelMain();

    // Should not reach here
    while (1)
        ;
    return 0;
}
