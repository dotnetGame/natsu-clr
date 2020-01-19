#pragma once
#include <Windows.h>
#include <stdexcept>

#define THROW_WIN32_IF_NOT(x)         \
    if (!(x))                         \
    {                                 \
        throw std::runtime_error(#x); \
    }

#define THROW_IF_FAILED(x)            \
    if (FAILED(x))                    \
    {                                 \
        throw std::runtime_error(#x); \
    }

inline FILETIME ticks_to_filetime(uint64_t ticks)
{
    ULARGE_INTEGER u;
    u.QuadPart = ticks;
    FILETIME ft { u.LowPart, u.HighPart };
    return ft;
}
