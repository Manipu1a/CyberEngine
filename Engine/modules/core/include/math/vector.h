#pragma once
#include "math/common.h"
#include "basic_math.hpp"
#include "platform/configure.h"
#include "cyber_core.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Math)

template <class T>
struct Vector3
{
    union
    {
        T m[3];
        struct
        {
            T x, y, z;
        };
        struct
        {
            T r, g, b; // Commonly used for color representation
        };
    };

    constexpr explicit Vector3(T value) noexcept : x(value), y(value), z(value) {}

    constexpr Vector3() noexcept : Vector3{0} {}

    constexpr Vector3(T x, T y, T z) noexcept : x(x), y(y), z(z) {}

};

template <class T>
struct Vector4
{
    union
    {
        T m[4];
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a; // Commonly used for color representation
        };
    };

    constexpr explicit Vector4(T value) noexcept : x(value), y(value), z(value), w(value) {}

    constexpr Vector4() noexcept : Vector4{0} {}

    constexpr Vector4(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w) {}
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
