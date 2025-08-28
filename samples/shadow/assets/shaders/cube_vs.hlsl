#include "common.hlsl"

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

cbuffer Constants
{
    float4x4 ProjectionMatrix;
}

void VSMain(in VSInput VSIn,
          out PSInput PSIn)
{
    PSIn.Pos = mul(ProjectionMatrix, float4(VSIn.pos, 1.0));
    PSIn.Normal = VSIn.normal;
    PSIn.UV = VSIn.uv;
}