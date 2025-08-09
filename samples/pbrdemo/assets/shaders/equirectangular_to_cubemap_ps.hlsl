

static const float PI = 3.14159265358979323846;

#ifndef NUM_PHI_SAMPLES
#define NUM_PHI_SAMPLES 64
#endif

#ifndef NUM_THETA_SAMPLES
#define NUM_THETA_SAMPLES 32
#endif

struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 LocalPos : TEXCOORD0;
};

float2 DirectionToSphericalUV(float3 direction)
{
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI;
    return float2(u, v);
}

const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(atan2(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv; 
}

Texture2D Environment_Texture;
SamplerState Texture_sampler;

void PSMain(in PSInput PSIn, out float4 Color : SV_Target)
{
    float3 N = normalize(PSIn.LocalPos.xyz);

    float3 up = float3(0.0f, 1.0f, 0.0f); // 假设上方向为Y轴
    float3 right = normalize(cross(up, N));

    float2 uv_sample = DirectionToSphericalUV(N);
    float3 color = Environment_Texture.Sample(Texture_sampler, uv_sample).rgb;

    Color = float4(color, 1.0f);
    //Color = float4(uv_sample, 0.0, 1.0); // For debugging, set color to normal
}