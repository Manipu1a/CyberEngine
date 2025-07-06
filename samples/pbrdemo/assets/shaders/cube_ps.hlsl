struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;

    float3 Tangent : TEXCOORD1; // Tangent vector
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

Texture2D BaseColor_Texture;
Texture2D Normal_Texture;

SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightDirection; // Color of the light
    float4 LightColor; // Position of the light in world space
};

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    float4 col = BaseColor_Texture.Sample(Texture_sampler, PSIn.UV);
    float3 normal = Normal_Texture.Sample(Texture_sampler, PSIn.UV).xyz;
    normal = normal * float3(2.0, 2.0, 1.0) - float3(1.0, 1.0, 0.0); // Convert normal from [0,1] to [-1,1]
    // TBN matrix
    float3 Bitangent = cross(PSIn.Normal, PSIn.Tangent);
    float3x3 TBN = float3x3(PSIn.Tangent, Bitangent, PSIn.Normal);
    normal = normalize(mul(TBN, normal));

    float NoL = dot(normal, normalize(LightDirection.xyz));
    //PSOut.Color = float4(normal, 1.0);
    PSOut.Color = col * NoL;
}