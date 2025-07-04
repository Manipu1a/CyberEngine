
struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

Texture2D Texture;
SamplerState Texture_sampler;

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{

    float4 col = Texture.Sample(Texture_sampler, PSIn.UV) * float4(PSIn.Color.rgb, 1.0);
    PSOut.Color = col;
}