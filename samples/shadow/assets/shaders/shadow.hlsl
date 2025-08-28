#include "common.hlsl"

cbuffer Constants
{
    float4x4 WorldViewProj;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
};

void main(in VSInput VSIn, out PSInput PSOut)
{
    PSOut.Pos = mul(WorldViewProj, float4(VSIn.pos, 1.0));
}