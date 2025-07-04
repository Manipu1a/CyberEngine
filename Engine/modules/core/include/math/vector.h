#pragma once
#include "math/common.h"
#include "basic_math.hpp"
#include "platform/configure.h"
#include "cyber_core.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Math)

template <class T>
struct Vector2
{
    union
    {
        T m[2];
        struct
        {
            T x, y;
        };
        struct
        {
            T r, g; // Commonly used for color representation
        };
    };

    constexpr explicit Vector2(T value) noexcept : x(value), y(value) {}

    constexpr Vector2() noexcept : Vector2{0} {}

    constexpr Vector2(T x, T y) noexcept : x(x), y(y) {}  
};

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

    constexpr Vector3 operator*(T scalar) const noexcept
    {
        return Vector3 {x * scalar, y * scalar, z * scalar};
    }

    constexpr Vector3& operator*=(T scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vector3& operator+=(const Vector3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    
    constexpr Vector3 operator<(const Vector3& other) const noexcept
    {
        return Vector3{ x < other.x ? static_cast<T>(1) : static_cast<T>(0),
                        y < other.y ? static_cast<T>(1) : static_cast<T>(0),
                        z < other.z ? static_cast<T>(1) : static_cast<T>(0)};
    }

    constexpr Vector3 operator<=(const Vector3& other) const noexcept
    {
        return Vector3{ x <= other.x ? static_cast<T>(1) : static_cast<T>(0),
                        y <= other.y ? static_cast<T>(1) : static_cast<T>(0),
                        z <= other.z ? static_cast<T>(1) : static_cast<T>(0)};
    }

    constexpr Vector3 operator>(const Vector3& other) const noexcept
    {
        return Vector3{ x > other.x ? static_cast<T>(1) : static_cast<T>(0),
                        y > other.y ? static_cast<T>(1) : static_cast<T>(0),
                        z > other.z ? static_cast<T>(1) : static_cast<T>(0)};
    }

    constexpr Vector3 operator>=(const Vector3& other) const noexcept
    {
        return Vector3{ x >= other.x ? static_cast<T>(1) : static_cast<T>(0),
                        y >= other.y ? static_cast<T>(1) : static_cast<T>(0),
                        z >= other.z ? static_cast<T>(1) : static_cast<T>(0)};
    }

    template <typename Y>
    constexpr static Vector3 make_vector(const Y& value) noexcept
    {
        return Vector3 {static_cast<T>(value[0]), static_cast<T>(value[1]), static_cast<T>(value[2])};
    }


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

template <class T>
constexpr Vector3<T>(min)(const Vector3<T>& a, const Vector3<T>& b) noexcept
{
    return Vector3<T>{ (std::min)(a.x, b.x), (std::min)(a.y, b.y), (std::min)(a.z, b.z) };
}

template <class T>
constexpr Vector3<T>(max)(const Vector3<T>& a, const Vector3<T>& b) noexcept
{
    return Vector3<T>{ (std::max)(a.x, b.x), (std::max)(a.y, b.y), (std::max)(a.z, b.z) };
}

template <class T>
constexpr Vector4<T>(min)(const Vector4<T>& a, const Vector4<T>& b) noexcept
{
    return Vector4<T>{ (std::min)(a.x, b.x), (std::min)(a.y, b.y), (std::min)(a.z, b.z), (std::min)(a.w, b.w) };
}
    
template <class T>
constexpr Vector4<T>(max)(const Vector4<T>& a, const Vector4<T>& b) noexcept
{
    return Vector4<T>{ (std::max)(a.x, b.x), (std::max)(a.y, b.y), (std::max)(a.z, b.z), (std::max)(a.w, b.w) };
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
