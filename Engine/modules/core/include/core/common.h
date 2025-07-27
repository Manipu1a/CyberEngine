#pragma once

#include <float.h>
#include <type_traits>
#include "log/Log.h"
#include <DirectXMath.h>

namespace Cyber
{
    // const value
    static constexpr float PI_ = 3.1415926535897932f;
    static constexpr float INV_PI_ = 0.31830988618f;

    static inline uint32_t round_up(uint32_t value, uint32_t multiple) { return (((value + multiple - 1) / multiple) * multiple);}
    static inline uint64_t round_up_64(uint64_t value, uint64_t multiple) { return (((value + multiple - 1) / multiple) * multiple);}

    static inline uint32_t round_down(uint32_t value, uint32_t multiple) { return value - value % multiple; }
    static inline uint64_t round_down_64(uint64_t value, uint64_t multiple) { return value - value % multiple; }
     
    template <typename T>
    bool is_power_of_two(T value)
    {
       return (value != 0) && ((value & (value - 1)) == 0);
    }

    template <typename T>
    inline constexpr bool is_aligned(T value, uint32_t alignment)
    {
       static_assert(std::is_integral<T>::value, "both types must be signed or unsigned");
       static_assert(!std::is_pointer<T>::value, "types must not be pointers");

       return (value & (alignment - 1)) == 0;
    }

    template <typename T>
    inline T align_up(T value, uint32_t alignment)
    {
       static_assert(std::is_integral<T>::value, "both types must be signed or unsigned");
       static_assert(!std::is_pointer<T>::value, "types must not be pointers");

       cyber_assert(is_power_of_two(alignment), "alignment must be a power of two");
       return (static_cast<T>(value) + static_cast<T>(alignment) - 1) & ~(static_cast<T>(alignment) - 1);
    }

    template <typename T>
    inline T align_down(T value, uint32_t alignment)
    {
       static_assert(std::is_integral<T>::value, "both types must be signed or unsigned");
       static_assert(!std::is_pointer<T>::value, "types must not be pointers");

       cyber_assert(is_power_of_two(alignment), "alignment must be a power of two");
       return static_cast<T>(value) & ~(static_cast<T>(alignment) - 1);
    }

    [[nodiscard]] static constexpr int32_t greatest_common_divisor(int32_t a, int32_t b) noexcept
    {
        while (b != 0)
        {
            int32_t t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    [[nodiscard]] static constexpr int32_t least_common_multiple(int32_t a, int32_t b) noexcept
    {
        auto gcd = greatest_common_divisor(a, b);
        if (gcd == 0) return 0; // Avoid division by zero
        return (a / gcd) * b;
    }

    template <typename Enum>
    constexpr bool any_flag_set(Enum value, Enum flag) noexcept
    {
        static_assert(std::is_enum<Enum>::value, "Enum must be an enumeration type");
        return (static_cast<std::underlying_type_t<Enum>>(value) & static_cast<std::underlying_type_t<Enum>>(flag)) != 0;
    }
}