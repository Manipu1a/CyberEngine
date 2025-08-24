#pragma once
#include "core/common.h"
#include "basic_math.hpp"
#include <cmath>
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

    static constexpr size_t get_component_count()
    {
        return 2;
    }

    T& operator[](size_t index)
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    const T& operator[](size_t index) const
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    bool operator==(const Vector2& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2& other) const
    {
        return !(*this == other);
    }

    T* data() { return reinterpret_cast<T*>(this); }

    const T* data() const { return reinterpret_cast<const T*>(this); }
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

    constexpr Vector3 operator*(const Vector3& right) const
    {
        return Vector3{x * right.x, y * right.y, z * right.z};
    }

    constexpr Vector3& operator*=(T scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vector3& operator*=(const Vector3& right)
    {
        x *= right.x;
        y *= right.y;
        z *= right.z;
        return *this;
    }

    constexpr Vector3& operator+=(const Vector3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    
    constexpr Vector3& operator-=(const Vector3& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vector3& operator/=(float scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
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

    constexpr Vector3 operator-(const Vector3& other) const
    {
        return Vector3{x - other.x, y - other.y, z - other.z}   ;
    }

    constexpr Vector3 operator-() const
    {
        return Vector3{-x, -y, -z};
    }

    bool operator==(const Vector3& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other) const
    {
        return !(*this == other);
    }

    static constexpr size_t get_component_count()
    {
        return 3;
    }

    T& operator[](size_t index)
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    const T& operator[](size_t index) const
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    template <typename Y>
    constexpr static Vector3 make_vector(const Y& value) noexcept
    {
        return Vector3 {static_cast<T>(value[0]), static_cast<T>(value[1]), static_cast<T>(value[2])};
    }

    T* data() { return reinterpret_cast<T*>(this); }

    const T* data() const { return reinterpret_cast<const T*>(this); }
};

template<class T>
static Vector3<T> normalize(const Vector3<T>& v)
{
    T length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0) return Vector3<T>{0, 0, 0};
    return Vector3<T>{v.x / length, v.y / length, v.z / length};
}

template<class T>
static Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

template <class T>
constexpr Vector3<T> operator*(T s, const Vector3<T>& v)
{
    return v * s;
}

template <class T>
constexpr Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>{a.x + b.x, a.y + b.y, a.z + b.z};
}

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

    constexpr Vector4(const Vector3<T>& vec, T w_value = 1) noexcept
        : x(vec.x), y(vec.y), z(vec.z), w(w_value) {}
    
    static constexpr size_t get_component_count()
    {
        return 4;
    }

    T& operator[](size_t index)
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    const T& operator[](size_t index) const
    {
        cyber_check(index < get_component_count());
        return m[index];
    }

    constexpr operator Vector3<T>() const noexcept
    {
        return Vector3<T>{x, y, z};
    }

    template <typename Y>
    constexpr static Vector4 make_vector(const Y& value) noexcept
    {
        return Vector4 {static_cast<T>(value[0]), static_cast<T>(value[1]), static_cast<T>(value[2]), static_cast<T>(value[3])};
    }

    T* data() { return reinterpret_cast<T*>(this); }

    const T* data() const { return reinterpret_cast<const T*>(this); }
};

template<class T>
static Vector4<T> normalize(const Vector4<T>& v)
{
    T length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (length == 0) return Vector4<T>{0, 0, 0, 0};
    return Vector4<T>{v.x / length, v.y / length, v.z / length, v.w / length};
}

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

template <class T>
constexpr T dot(const Vector2<T>& a, const Vector2<T>& b) noexcept
{
    return a.x * b.x + a.y * b.y;
}

template <class T>
constexpr T dot(const Vector3<T>& a, const Vector3<T>& b) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T>
constexpr T dot(const Vector4<T>& a, const Vector4<T>& b) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template <class VectorType>
constexpr auto length(const VectorType& v) noexcept
{
    return std::sqrt(dot(v, v));
}

template <class T>
constexpr Vector2<T> abs(const Vector2<T>& v) noexcept
{
    return Vector2<T>{ (v.x >= 0) ? v.x : -v.x, 
        (v.y >= 0) ? v.y : -v.y };
}

template <class T>
constexpr Vector3<T> abs(const Vector3<T>& v) noexcept
{
    return Vector3<T>{ (v.x >= 0) ? v.x : -v.x, 
        (v.y >= 0) ? v.y : -v.y,
        (v.z >= 0) ? v.z : -v.z };
}

template <class T>
constexpr Vector4<T> abs(const Vector4<T>& v) noexcept
{
    return Vector4<T>{ (v.x >= 0) ? v.x : -v.x, 
        (v.y >= 0) ? v.y : -v.y,
        (v.z >= 0) ? v.z : -v.z,
        (v.w >= 0) ? v.w : -v.w };
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
