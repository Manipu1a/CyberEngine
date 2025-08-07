#ifndef USE_NORMAL_MAP
#define USE_NORMAL_MAP 0
#endif
static const float PI = 3.14159265358979323846;

struct PSInput
{
    float4 ClipPos : SV_POSITION;
    float4 WorldPos : TEXCOORD0;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD1;
    float3 Tangent : TEXCOORD2;
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

struct BRDFContext
{
    float NoV;
	float NoL;
	float VoL;
	float NoH;
	float VoH;
};

Texture2D BaseColor_Texture;
Texture2D MetallicRoughness_Texture;
Texture2D Normal_Texture;

TextureCube Environment_Texture;

SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightDirection; // Color of the light
    float4 LightColor; // Position of the light in world space
    float4 CameraPosition;
};

void InitContext(inout BRDFContext Context, float3 N, float3 V, float3 L)
{
    Context.NoL = dot(N, L);
    Context.NoV = dot(N, V);
    Context.VoL = dot(V, L);
    float InvLengthH = rsqrt(2 + 2 * Context.VoL);
    Context.NoH = saturate( (Context.NoL + Context.NoV) * InvLengthH);
    Context.VoH = saturate( InvLengthH + InvLengthH * Context.VoL);
}

float3 Diffuse_Lambert(float3 DiffuseColor)
{
    return DiffuseColor * (1.0 / PI);
}

float DistributonGGX(float3 NoH, float roughness)
{
    float a2 = roughness * roughness;

    float NoH2 = NoH * NoH;

    float nom = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float GeometrySchlickGGX(float NoV, float k)
{
    float nom = NoV;
    float denom = NoV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(float NoV, float NoL, float3 L, float k)
{
    float ggx1 = GeometrySchlickGGX(NoV, k);
    float ggx2 = GeometrySchlickGGX(NoL, k);
    return ggx1 * ggx2;
}

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    float4 base_color = BaseColor_Texture.Sample(Texture_sampler, PSIn.UV);
    float4 metallic_roughness = MetallicRoughness_Texture.Sample(Texture_sampler, PSIn.UV);
#if USE_NORMAL_MAP
    float3 normal = Normal_Texture.Sample(Texture_sampler, PSIn.UV).xyz;
    normal = normal * float3(2.0, 2.0, 1.0) - float3(1.0, 1.0, 0.0); // Convert normal from [0,1] to [-1,1]
    normal = normalize(normal);
    // TBN matrix
    float3 Bitangent = cross(PSIn.Normal, PSIn.Tangent);
    float3x3 TBN = float3x3(PSIn.Tangent, Bitangent, PSIn.Normal);
    normal = normalize(mul(TBN, normal));
#else
    float3 normal = normalize(PSIn.Normal); // Use the normal from the vertex shader
#endif

    BRDFContext Context;
    float3 V = normalize(CameraPosition.xyz - PSIn.WorldPos.xyz);
    float3 L = normalize(LightDirection.xyz - PSIn.WorldPos.xyz);
    InitContext(Context, normal, V, L);
    float D = DistributonGGX(Context.NoH, metallic_roughness.y);
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, base_color.xyz, metallic_roughness.x);
    float3 F = FresnelSchlick(Context.VoH, F0);

    float diff = max(dot(normal, L), 0.0);
    float3 diffuse = diff * Diffuse_Lambert(base_color.xyz);
    float4 environment_color = Environment_Texture.Sample(Texture_sampler, normal);
    
    float NoL = Context.NoL;
    float NoV = dot(normal, V);
    float3 specular = pow(max(Context.NoH, 0.0), 32.0);
    
    float3 FinalColor = (diffuse + specular) * LightColor.xyz;
    PSOut.Color = float4(FinalColor, 1.0f);
}