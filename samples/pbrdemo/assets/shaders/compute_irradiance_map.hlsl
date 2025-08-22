
#include "precompute_common.hlsl"

static const uint NUM_SAMPLES = 1024;

#ifndef NUM_PHI_SAMPLES
#define NUM_PHI_SAMPLES 64
#endif

#ifndef NUM_THETA_SAMPLES
#define NUM_THETA_SAMPLES 32
#endif

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS;  // 添加这一行
};


float2 DirectionToSphericalUV(float3 direction)
{
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI;
    return float2(u, v);
}

TextureCube Environment_Texture;
SamplerState Texture_sampler;

void PSMain(in VSOutput PSIn, out float4 Color : SV_Target)
{
    float3 N = normalize(PSIn.ClipPos.xyz);
    float3 T = normalize(cross(N, abs(N.y) > 0.5 ? float3(1.0, 0.0, 0.0) : float3(0.0, 1.0, 0.0)));
    float3 B = normalize(cross(N, T));

    float3 irradiance = float3(0.0, 0.0, 0.0);
    for(uint i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 Xi = Hammersley(i, NUM_SAMPLES);

        float3 L;
        float pdf;
        SampleDirectionCosineHemisphere(Xi, L, pdf);
        L = normalize(L.x * T + L.y * B + L.z * N);

        irradiance += Environment_Texture.Sample(Texture_sampler, L).rgb;
    }

    Color = float4(irradiance / NUM_SAMPLES, 1.0f);
}