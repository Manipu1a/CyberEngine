struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 WorldPos : TEXCOORD1;
    float ViewDepth : TEXCOORD2;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

Texture2D Texture;
Texture2DArray ShadowTexture;

SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightDirection;
    float4 LightColor;
};

#define MAX_CASCADE_COUNT 4

cbuffer CSMConstants
{
    float4x4 CascadeViewProj[MAX_CASCADE_COUNT];
    float4 CascadeSplits;    // view-space split distances
    float4 ShadowParams;     // x=resolution, y=1/resolution, z=cascade_count, w=depth_bias
    float4 DebugParams;      // x=show_cascade_debug
};

uint SelectCascade(float viewDepth)
{
    uint cascadeCount = (uint)ShadowParams.z;
    float splits[MAX_CASCADE_COUNT] = { CascadeSplits.x, CascadeSplits.y, CascadeSplits.z, CascadeSplits.w };

    for (uint i = 0; i < cascadeCount; ++i)
    {
        if (viewDepth < splits[i])
            return i;
    }
    return cascadeCount - 1;
}

float CalcCSMShadowFactor(float3 worldPos, float viewDepth)
{
    uint cascade = SelectCascade(viewDepth);

    float4 shadowPos = mul(float4(worldPos, 1.0), CascadeViewProj[cascade]);
    float3 projCoords = shadowPos.xyz / shadowPos.w;

    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = -projCoords.y * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0;
    }

    float bias = ShadowParams.w;
    float currentDepth = projCoords.z - bias;

    float shadow = 0.0;
    float2 texelSize = float2(ShadowParams.y, ShadowParams.y);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float shadowSample = ShadowTexture.Sample(
                Texture_sampler,
                float3(projCoords.xy + float2(x, y) * texelSize, (float)cascade)
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

    float3 lightDir = normalize(LightDirection.xyz);
    float3 normal = normalize(PSIn.Normal);

    float NdotL = max(dot(normal, lightDir), 0.0);

    float shadowFactor = CalcCSMShadowFactor(PSIn.WorldPos, PSIn.ViewDepth);

    float3 ambient = 0.2 * col.rgb;
    float3 diffuse = shadowFactor * NdotL * LightColor.rgb * col.rgb;

    float3 finalColor = ambient + diffuse;

    // Debug cascade visualization
    if (DebugParams.x > 0.5)
    {
        uint cascade = SelectCascade(PSIn.ViewDepth);
        float3 cascadeColors[MAX_CASCADE_COUNT] = {
            float3(1.0, 0.2, 0.2),
            float3(0.2, 1.0, 0.2),
            float3(0.2, 0.2, 1.0),
            float3(1.0, 1.0, 0.2)
        };
        finalColor = lerp(finalColor, cascadeColors[cascade], 0.3);
    }

    PSOut.Color = float4(finalColor, col.a);
}
