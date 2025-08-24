#pragma once
#include "matrix.h"
#include "vector.h"

using float4x4 = Cyber::Math::Matrix4x4<float>;
using float4 = Cyber::Math::Vector4<float>;
using float3 = Cyber::Math::Vector3<float>;
using float2 = Cyber::Math::Vector2<float>;
using quaternion_f = Cyber::Math::Quaternion<float>;
using quaternion_d = Cyber::Math::Quaternion<double>;

static constexpr double PI = 3.14159265358979323846;
static constexpr float PI_F = 3.1415927f;