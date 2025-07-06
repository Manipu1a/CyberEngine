struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

Texture2D Texture;
SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightColor; // Color of the light
    float4 LightPosition; // Position of the light in world space
};

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    float4 col = Texture.Sample(Texture_sampler, PSIn.UV);
    PSOut.Color = col;
}