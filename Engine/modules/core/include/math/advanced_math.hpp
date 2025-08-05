#pragma once
#include "basic_math.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)

struct BoundBox
{
    float3 Min;
    float3 Max;
    
    BoundBox Transform( const float4x4& m) const
    {
        BoundBox NewBB;
        NewBB.Min = float3::make_vector(m[3]);
        NewBB.Max = NewBB.Min;

        float3 v0, v1;

        float3 right = float3::make_vector(m[0]);

        v0 = right * Min.x;
        v1 = right * Max.x;
        
        NewBB.Min += Math::min(v0, v1);
        NewBB.Max += Math::max(v0, v1);

        float3 up = float3::make_vector(m[1]);

        v0 = up * Min.y;
        v1 = up * Max.y;
        NewBB.Min += Math::min(v0, v1);
        NewBB.Max += Math::max(v0, v1);

        float3 back = float3::make_vector(m[2]);

        v0 = back * Min.z;
        v1 = back * Max.z;
        NewBB.Min += Math::min(v0, v1);
        NewBB.Max += Math::max(v0, v1);

        return NewBB;
    };
};

CYBER_END_NAMESPACE