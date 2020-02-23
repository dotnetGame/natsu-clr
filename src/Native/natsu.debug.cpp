#include "Chino.Kernel.h"
#include "System.Private.CoreLib.h"
#ifdef _MSC_VER
#include <intrin.h>
#endif

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;
using namespace System_Private_CoreLib::System::Diagnostics;

void Debug::_s_WriteCore(gc_obj_ref<String> message)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
}

void Debug::_s_WriteLineCore(gc_obj_ref<String> message)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(Environment::_s_get_NewLine());
}

void Debug::_s_FailCore(gc_obj_ref<String> message, gc_obj_ref<String> detailMessage)
{
    Chino_Kernel::Chino::Kernel::KernelDebug::_s_Write(message);
#ifdef _MSC_VER
    __debugbreak();
#else
    assert(false);
#endif
}
