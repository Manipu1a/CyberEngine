#pragma once
#include "math/common.h"
#include "vector.h"
#include <cmath>
#include <algorithm>
#include "platform/configure.h"
#include "cyber_core.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Math)

template<class T>
struct Matrix4x4
{
    union
    {
        T m[4][4];
        struct
        {
            T m00, m01, m02, m03;
            T m10, m11, m12, m13;
            T m20, m21, m22, m23;
            T m30, m31, m32, m33;
        };
    };

    constexpr explicit Matrix4x4(T value) noexcept :
        m00(value), m01(value), m02(value), m03(value),
        m10(value), m11(value), m12(value), m13(value),
        m20(value), m21(value), m22(value), m23(value),
        m30(value), m31(value), m32(value), m33(value)
    {

    }

    constexpr Matrix4x4() noexcept : 
        Matrix4x4{0} {}
    
    constexpr Matrix4x4(T m00, T m01, T m02, T m03,
                        T m10, T m11, T m12, T m13,
                        T m20, T m21, T m22, T m23,
                        T m30, T m31, T m32, T m33) noexcept :
        m00(m00), m01(m01), m02(m02), m03(m03),
        m10(m10), m11(m11), m12(m12), m13(m13),
        m20(m20), m21(m21), m22(m22), m23(m23),
        m30(m30), m31(m31), m32(m32), m33(m33)
    {

    }

    constexpr Matrix4x4(const Vector4<T>& row0,
                        const Vector4<T>& row1,
                        const Vector4<T>& row2,
                        const Vector4<T>& row3) noexcept :
        m00(row0.x), m01(row0.y), m02(row0.z), m03(row0.w),
        m10(row1.x), m11(row1.y), m12(row1.z), m13(row1.w),
        m20(row2.x), m21(row2.y), m22(row2.z), m23(row2.w),
        m30(row3.x), m31(row3.y), m32(row3.z), m33(row3.w)
    {

    }

    constexpr bool operator==(const Matrix4x4& r) const
    {
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                if (m[i][j] != r.m[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    constexpr bool operator!=(const Matrix4x4& r) const
    {
        return !(*this == r);
    }


    T* operator[](size_t row)
    {
        return m[row];
    }

    const T* operator[](size_t row) const
    {
        return m[row];
    }
    
    T* Data() { return (*this)[0]; }
    const T* Data() const { return (*this)[0]; }

    Matrix4x4& operator*=(T s)
    {
        for(int i = 0; i < 16; ++i)
        {
            (reinterpret_cast<T*>(this))[i] *= s;
        }
        return *this;
    }

    Matrix4x4& operator*=(const Matrix4x4& right)
    {
        *this = mul(*this, right);
        return *this;
    }

    
    // Returns the transpose of the matrix
    constexpr Matrix4x4 transpose() const
    {
        return Matrix4x4(
            m00, m10, m20, m30,
            m01, m11, m21, m31,
            m02, m12, m22, m32,
            m03, m13, m23, m33
        );
    }

    constexpr static Matrix4x4 translation(T x, T y, T z)
    {
        return Matrix4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            x, y, z, 1
        );
    }

    constexpr static Matrix4x4 scale(T x, T y, T z)
    {
        return Matrix4x4(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
    }

    constexpr static Matrix4x4 scale(T x)
    {
        return Matrix4x4(
            x, 0, 0, 0,
            0, x, 0, 0,
            0, 0, x, 0,
            0, 0, 0, 1
        );
    }

    constexpr static Matrix4x4 scale(const Vector3<T>& v)
    {
        return scale(v.x, v.y, v.z);
    }

    // d3d style rotation matrices
    // angle (in radians) is clockwise when looking down the axis towards the origin
    constexpr static Matrix4x4 RotationX(T angle_in_radians)
    {
        T c = std::cos(angle_in_radians);
        T s = std::sin(angle_in_radians);

        return Matrix4x4(
            1, 0, 0, 0,
            0, c, s, 0,
            0, -s,c, 0,
            0, 0, 0, 1
        );
    }

    constexpr static Matrix4x4 RotationY(T angle_in_radians)
    {
        T c = std::cos(angle_in_radians);
        T s = std::sin(angle_in_radians);

        return Matrix4x4(
            c, 0, -s, 0,
            0, 1, 0, 0,
            s, 0, c, 0,
            0, 0, 0, 1
        );
    }

    constexpr static Matrix4x4 RotationZ(T angle_in_radians)
    {
        T c = std::cos(angle_in_radians);
        T s = std::sin(angle_in_radians);

        return Matrix4x4(
            c, s, 0, 0,
            -s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    constexpr static Matrix4x4 translation(const Vector3<T>& v)
    {
        return translation(v.x, v.y, v.z);
    }

    constexpr static Matrix4x4 Identity()
    {
        return Matrix4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4x4 mul(const Matrix4x4& left, const Matrix4x4& right)
    {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                for(int k = 0; k < 4; ++k)
                {
                    result.m[i][j] += left.m[i][k] * right.m[k][j];
                }
            }
        }
        return result;
    }

};

template<class T>
constexpr Matrix4x4<T> operator*(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
{
    return Matrix4x4<T>::mul(lhs, rhs);
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
