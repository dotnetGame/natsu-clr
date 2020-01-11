// chino os runtime
#pragma once
#include "Generated/System.Private.CoreLib.h"

namespace Chino_Core
{
template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> Chino::IOVolatile::_s_As(uint32_t address)
{
    return ::natsu::gc_ref_from_addr<::natsu::variable_type_t<T>>((uintptr_t)address);
}

template <class T>
::natsu::variable_type_t<T> Chino::IOVolatile::_s_Read(::natsu::gc_ref<::natsu::variable_type_t<T>> address)
{
    using elem_t = std::decay_t<decltype(address->Value)>;
    auto value = *reinterpret_cast<const volatile elem_t *>(&address->Value);
    T result;
    result.Value = value;
    return result;
}

template <class T>
void Chino::IOVolatile::_s_Write(::natsu::gc_ref<::natsu::variable_type_t<T>> address, ::natsu::variable_type_t<T> value)
{
    using elem_t = std::decay_t<decltype(address->Value)>;
    auto ptr = reinterpret_cast<volatile elem_t *>(&address->Value);
    *ptr = value.Value;
}
}
