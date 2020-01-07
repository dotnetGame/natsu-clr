// chino os runtime
#pragma once
#include "Generated/System.Private.CorLib.h"

namespace Chino_Core
{
template <class T>
::natsu::gc_ref<::natsu::variable_type_t<T>> Chino::IOVolatile::_s_As(::System_Runtime::System::UInt32 address)
{
    return ::natsu::gc_ref<::natsu::variable_type_t<T>>((uintptr_t)address.m_value);
}

template <class T>
::natsu::variable_type_t<T> Chino::IOVolatile::_s_Read(::natsu::gc_ref<::natsu::variable_type_t<T>> address)
{
    using elem_t = std::decay_t<decltype(address->Value.m_value)>;
    auto value = *reinterpret_cast<const volatile elem_t *>(&address->Value.m_value);
    T result;
    result.Value = value;
    return result;
}

template <class T>
void Chino::IOVolatile::_s_Write(::natsu::gc_ref<::natsu::variable_type_t<T>> address, ::natsu::variable_type_t<T> value)
{
    using elem_t = std::decay_t<decltype(address->Value.m_value)>;
    auto ptr = reinterpret_cast<volatile elem_t *>(&address->Value.m_value);
    *ptr = value.Value.m_value;
}
}
