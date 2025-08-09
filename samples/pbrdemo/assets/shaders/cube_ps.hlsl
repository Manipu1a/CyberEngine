#ifndef USE_NORMAL_MAP
#define USE_NORMAL_MAP 0
#endif
static const float PI = 3.14159265358979323846;

struct PSInput
{
    float4 ClipPos : SV_POSITION;
    float3 WorldPos : WORLD_POS;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
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

TextureCube Irradiance_Texture;
SamplerState Texture_sampler;

cbuffer LightingConstants
{
    float4 LightDirection; // Color of the light
    float4 LightColor; // Position of the light in world space
    float4 CameraPosition;
};

void InitContext(inout BRDFContext Context, float3 N, float3 V, float3 L)
{
    Context.NoL = max(dot(N, L), 0.0);
    Context.NoV = max(dot(N, V), 0.0);
    Context.VoL = max(dot(V, L), 0.0);
    float InvLengthH = rsqrt(2 + 2 * Context.VoL);
    Context.NoH = saturate( (Context.NoL + Context.NoV) * InvLengthH);
    Context.VoH = saturate( InvLengthH + InvLengthH * Context.VoL);
}

float3 Diffuse_Lambert(float3 DiffuseColor)
{
    return DiffuseColor * (1.0 / PI);
}
// GGX / Trowbridge-Reitz normal distribution function
float DistributonGGX(float NoH, float roughness)
{
    float a2 = roughness * roughness;
    float NoH2 = NoH * NoH;

    float nom = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// Fresnel-Schlick approximation with roughness
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 a = 1.0 - roughness;
    return F0 + (max(a, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Schlick's approximation for geometry term
float GeometrySchlickGGX(float NoV, float roughness)
{
    float nom = NoV;
    float denom = NoV * (1.0 - roughness) + roughness;
    return nom / denom;
}
// Smith's method for geometry term
float GeometrySmith(float NoV, float NoL, float roughness)
{
    float ggx1 = GeometrySchlickGGX(NoV, roughness);
    float ggx2 = GeometrySchlickGGX(NoL, roughness);
    return ggx1 * ggx2;
}

void PSMain(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    float4 base_color = BaseColor_Texture.Sample(Texture_sampler, PSIn.UV);
    float4 metallic_roughness = MetallicRoughness_Texture.Sample(Texture_sampler, PSIn.UV);
    float metallic = metallic_roughness.y;
    float roughness = metallic_roughness.z;
    
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
    float3 L = normalize(LightDirection.xyz);
    InitContext(Context, normal, V, L);
    float NDF = DistributonGGX(Context.NoH, roughness);
    float G = GeometrySmith(Context.NoV, Context.NoL, roughness);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, base_color.xyz, metallic);
    float3 F = FresnelSchlick(Context.VoH, F0);
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * Context.NoV * Context.NoL + 0.001;
    float3 specular = numerator / denominator;

    float3 kD = 1.0 - F; // Fresnel term == Ks
    kD *= 1.0 - metallic;

    float3 diffuse = kD * Diffuse_Lambert(base_color.xyz);
    float3 irradiance = Irradiance_Texture.Sample(Texture_sampler, normal).xyz;
    float3 indirect_ks = FresnelSchlickRoughness(Context.NoV, F0, roughness);
    float3 indirect_kd = 1.0 - indirect_ks;

    float3 indirect_diffuse = irradiance * base_color.xyz;
    float3 ambient =  indirect_diffuse;

    float3 FinalColor = (diffuse + specular) * LightColor.xyz * Context.NoL;
    FinalColor += ambient;
    //FinalColor = FinalColor / (FinalColor + float3(1.0, 1.0, 1.0));
    float3 gamma = float3(1.0/2.2, 1.0/2.2, 1.0/2.2);
    //FinalColor = pow(FinalColor, gamma); // Gamma correction
    PSOut.Color = float4(FinalColor, 1.0f);
}