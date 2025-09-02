struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 WorldPos : TEXCOORD1;
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

Texture2D Texture;
Texture2D ShadowTexture;

SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightColor; // Color of the light
    float4 LightPosition; // Position of the light in world space
};

cbuffer ViewConstants
{
    float4x4 ModelMatrix;
    float4x4 ViewProjectionMatrix;
    float4x4 ShadowMatrix;
};

float CalcShadowFactor(float4 shadowPos)
{
    float3 projCoords = shadowPos.xyz / shadowPos.w;
    
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = -projCoords.y * 0.5 + 0.5;
    
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0;
    }
    
    float bias = 0.005;
    float currentDepth = projCoords.z - bias;
    
    float shadow = 0.0;
    float2 texelSize = 1.0 / float2(2048.0, 2048.0);
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float shadowSample = ShadowTexture.Sample(
                Texture_sampler,
                projCoords.xy + float2(x, y) * texelSize           
            ).r;
            shadow += currentDepth < shadowSample ? 1.0 : 0.0;
        }
    }
    
    return shadow / 9.0;
}

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    float4 col = Texture.Sample(Texture_sampler, PSIn.UV);
    
    float3 lightDir = normalize(LightPosition.xyz - PSIn.WorldPos.xyz);
    float3 normal = normalize(PSIn.Normal);
    
    float NdotL = max(dot(normal, lightDir), 0.0);

    float4 shadowPos = mul(float4(PSIn.WorldPos, 1.0), ShadowMatrix);
    float shadowFactor = CalcShadowFactor(shadowPos);
    
    float3 ambient = 0.2 * col.rgb;
    float3 diffuse = shadowFactor * NdotL * LightColor.rgb * col.rgb;
    
    PSOut.Color = float4(ambient + diffuse, col.a);
}