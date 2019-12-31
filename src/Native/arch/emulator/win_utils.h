#pragma once
#include <Windows.h>
#include <stdexcept>

#define THROW_WIN32_IF_NOT(x)         \
    if (!(x))                         \
    {                                 \
        throw std::runtime_error(#x); \
    }
