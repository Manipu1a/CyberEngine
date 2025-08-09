struct VSInput
{
    float3 pos : ATTRIB0;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 LocalPos : TEXCOORD0;
};

cbuffer Constants
{
    float4x4 ModelMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
}

void VSMain(in VSInput input, out PSInput output)
{
    float4x4 worldViewProj = mul(ProjectionMatrix, mul(ViewMatrix, ModelMatrix));
    float4 clipSpacePos = mul(worldViewProj, float4(input.pos, 1.0));
    output.Pos = clipSpacePos.xyww;
    output.LocalPos = float4(input.pos, 1.0);
}