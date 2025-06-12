#pragma once
#include "platform/configure.h"

CYBER_BEGIN_NAMESPACE(Cyber)

struct Windows_Misc
{
    inline static uint32_t count_one_bits(uint32_t value)
    {
        auto bits = __popcnt(value);
        return bits;
    }
};

CYBER_END_NAMESPACE
