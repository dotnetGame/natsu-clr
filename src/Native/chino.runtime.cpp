#include "Chino.Kernel.h"
#include "Chino.Interop.h"
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
using namespace Chino_Core::Chino::Objects;
using namespace Chino_IO::Chino::IO;
using namespace Chino_IO::Chino::IO::Devices;
using namespace Chino_Kernel::Chino;
using namespace Chino_Kernel::Chino::Services;
using namespace Chino_Interop;
using namespace Chino_Interop::Chino;
using namespace System_Private_CoreLib::System;

namespace
{
gc_obj_ref<SafeFileHandle> make_handle(gc_obj_ref<> obj)
{
    auto handle = make_object<SafeFileHandle>();
    handle->_accessor = obj.cast<IDisposable>();
    return handle;
}

gc_obj_ref<Accessor_1<Device>> get_device(gc_obj_ref<SafeFileHandle> handle)
{
    check_null_obj_ref(handle);
    auto accessor = handle->_accessor.cast<Accessor_1<Device>>();
    check_null_obj_ref(accessor);
    return accessor;
}
}

void KernelServiceHost::UserAppMain(gc_obj_ref<KernelServiceHost> _this)
{
    CHINO_APP_MODULE::CHINO_APP_NAMESPACE::Program::_s_Main(nullptr);
}

gc_obj_ref<SafeFileHandle> Interop_IO::_s_get_StdinHandle()
{
    return make_handle(ConsoleHostService::get_Stdin(ConsoleHostService::_s_get_Current()));
}

gc_obj_ref<SafeFileHandle> Interop_IO::_s_get_StdoutHandle()
{
    return make_handle(ConsoleHostService::get_Stdout(ConsoleHostService::_s_get_Current()));
}

gc_obj_ref<SafeFileHandle> Interop_IO::_s_get_StderrHandle()
{
    return make_handle(ConsoleHostService::get_Stderr(ConsoleHostService::_s_get_Current()));
}

bool Interop_IO::_s_IsConsoleHandle(gc_obj_ref<SafeFileHandle> handle)
{
    return (bool)ops::isinst<ConsoleDevice>(get_device(handle)->_object);
}

bool Interop_IO::_s_StdinReady()
{
    return true;
}

int32_t Interop_IO::_s_Read(gc_obj_ref<SafeFileHandle> handle, Span_1<uint8_t> buffer)
{
    auto device = get_device(handle);
    return vtable<Device>(device->_object).Read_System__Span_1_System__Byte(device->_object, buffer);
}

void Interop_IO::_s_Write(gc_obj_ref<SafeFileHandle> handle, ReadOnlySpan_1<uint8_t> buffer)
{
    auto device = get_device(handle);
    return vtable<Device>(device->_object).Write_System__ReadOnlySpan_1_System__Byte(device->_object, buffer);
}

int32_t Interop_Text::_s_SNPrintF(gc_ptr<uint8_t> str, int32_t size, gc_obj_ref<SZArray_1<uint8_t>> format, gc_obj_ref<SZArray_1<uint8_t>> arg1)
{
    return std::snprintf(reinterpret_cast<char *>(str.ptr_), size, reinterpret_cast<const char *>(format->elements_), reinterpret_cast<const char *>(arg1->elements_));
}

int32_t Interop_Text::_s_SNPrintF(gc_ptr<uint8_t> str, int32_t size, gc_obj_ref<SZArray_1<uint8_t>> format, int32_t arg1)
{
    return std::snprintf(reinterpret_cast<char *>(str.ptr_), size, reinterpret_cast<const char *>(format->elements_), arg1);
}
