#include "Chino.Kernel.h"
#include <cmath>
#include <cstring>

using namespace natsu;
using namespace Chino_Core::Chino;
using namespace System_Runtime::System;

void SystemServices::_s_Initialize()
{
    auto &s_fields = static_holder<typename SystemServices::Static>::get();
    //s_fields._scheduler = null;
}
