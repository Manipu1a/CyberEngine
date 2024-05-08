
#ifndef MANUAL_SRGB_CONVERSION
    #define MANUAL_SRGB_CONVERSION 0
#endif

float GAMMA_TO_LINEAR(float Gamma)
{
    return ((Gamma) < 0.04045 ? (Gamma) / 12.92 : pow(max((Gamma) + 0.055, 0.0) / 1.055, 2.4));
}

void SRGBA_TO_LINEAR(inout float4 col)
{
#if MANUAL_SRGB_CONVERSION
    col.r = GAMMA_TO_LINEAR(col.r);
    col.g = GAMMA_TO_LINEAR(col.g);
    col.b = GAMMA_TO_LINEAR(col.b);
    col.a = 1.0 - GAMMA_TO_LINEAR(1.0 - col.a);
#else
    col.rgb = col.rgb;
#endif
}

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

Texture2D    Texture;
SamplerState Texture_sampler;

float4 main(in PSInput PSIn) : SV_Target
{
    float4 col = Texture.Sample(Texture_sampler, PSIn.uv) * PSIn.col;
    col.rgb *= col.a;
    SRGBA_TO_LINEAR(col);
    return col;
}