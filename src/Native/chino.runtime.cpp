#include "Chino.Kernel.h"
#include <cmath>
#include <cstring>

// clang-format off
#define STRINGFY(a) #a
#define STRINGFY_CONCAT(a, b) STRINGFY(a##b##.h)
#define CHINO_APP_H(name) STRINGFY_CONCAT(Generated/, name)
#define STRINGFY(a) #a
// clang-format on

#include CHINO_APP_H(CHINO_APP)

using namespace natsu;
using namespace Chino_Core::Chino;
using namespace Chino_Kernel::Chino;
using namespace Chino_Kernel::Chino::Services;
using namespace System_Runtime::System;

void KernelServiceHost::UserAppMain(gc_obj_ref<KernelServiceHost> _this)
{
    CHINO_APP_MODULE::CHINO_APP_NAMESPACE::Program::_s_Main(nullptr);
}
