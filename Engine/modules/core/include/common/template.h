#pragma once
#include "cyber_core.config.h"
#include "platform/configure.h"
#include <cstdint>

template<typename T>
T Bitmask(uint32_t count);

template<>
CYBER_FORCE_INLINE uint64_t Bitmask<uint64_t>(uint32_t count)
{
    return ((uint64_t(count < 64)) << count) - 1;
}

template<>
CYBER_FORCE_INLINE uint32_t Bitmask<uint32_t>(uint32_t count)
{
    return (uint32_t(uint64_t(1) << count)) - 1;
}

template<>
CYBER_FORCE_INLINE uint16_t Bitmask<uint16_t>(uint32_t count)
{
    return (uint16_t(uint32_t(1) << count)) - 1;
}

template<>
CYBER_FORCE_INLINE uint8_t Bitmask<uint8_t>(uint32_t count)
{
    return (uint8_t(uint16_t(1) << count)) - 1;
}
