#pragma once

#include <float.h>

namespace Cyber
{
    // const value
    static constexpr float PI_ = 3.1415926535897932f;
    static constexpr float INV_PI_ = 0.31830988618f;

    static inline uint32_t round_up(uint32_t value, uint32_t multiple) { return (((value + multiple - 1) / multiple) * multiple);}
    static inline uint64_t round_up_64(uint64_t value, uint64_t multiple) { return (((value + multiple - 1) / multiple) * multiple);}

    static inline uint32_t round_down(uint32_t value, uint32_t multiple) { return value - value % multiple; }
    static inline uint64_t round_down_64(uint64_t value, uint64_t multiple) { return value - value % multiple; }
     
}