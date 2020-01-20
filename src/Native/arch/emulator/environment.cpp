#include "System.Private.CoreLib.h"
#include "System.Runtime.Extensions.h"
#include "win_utils.h"
#include <Windows.h>
#include <algorithm>
#include <bitset>
#include <vector>

using namespace natsu;
using namespace System_Private_CoreLib::System;
using namespace System_Runtime_Extensions::System::Diagnostics;

int32_t Environment::_s_GetProcessorCount()
{
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info;
    DWORD buffer_len = 0;

    while (!GetLogicalProcessorInformation(info.data(), &buffer_len))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            info.resize(buffer_len);
    }

    int32_t logical_core_count = 0;
    for (auto &item : info)
    {
        if (item.Relationship == RelationProcessorCore)
        {
            std::bitset<sizeof(item.ProcessorMask) * 8> core_bits(item.ProcessorMask);
            logical_core_count += core_bits.count();
        }
    }

    return logical_core_count;
}

int64_t Stopwatch::_s_QueryPerformanceFrequency()
{
    int64_t value;
    THROW_WIN32_IF_NOT(QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&value)));
    return value;
}

int64_t Stopwatch::_s_QueryPerformanceCounter()
{
    int64_t value;
    THROW_WIN32_IF_NOT(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&value)));
    return value;
}
