#include "common.hlsl"

#define MAX_CASCADE_COUNT 4

cbuffer ShadowMVPConstants
{
    float4x4 ShadowMVPs[MAX_CASCADE_COUNT];
};

cbuffer CascadeIndex
{
    uint CascadeIdx;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
};

void main(in VSInput VSIn, out PSInput PSOut)
{
    PSOut.Pos = mul(float4(VSIn.pos, 1.0), ShadowMVPs[CascadeIdx]);
}
