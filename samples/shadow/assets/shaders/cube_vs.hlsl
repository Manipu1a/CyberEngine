#include "common.hlsl"

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 WorldPos : TEXCOORD1;
};

cbuffer ViewConstants
{
    float4x4 ModelMatrix;
    float4x4 ViewProjectionMatrix;
    float4x4 ShadowMatrix;
};

void VSMain(in VSInput VSIn,
          out PSInput PSIn)
{
    float4 worldPos = mul(float4(VSIn.pos, 1.0), ModelMatrix);
    PSIn.Pos  = mul(float4(VSIn.pos, 1.0), ViewProjectionMatrix);
    PSIn.WorldPos = worldPos.xyz / worldPos.w;
    PSIn.Normal = normalize(mul(float4(VSIn.normal, 0.0), ModelMatrix).xyz);
    PSIn.UV = VSIn.uv;
}