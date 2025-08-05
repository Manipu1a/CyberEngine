#ifndef USE_NORMAL_MAP
#define USE_NORMAL_MAP 0
#endif

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
    float3 V = normalize(CameraPosition.xyz);
    float3 L = normalize(LightDirection.xyz);
    InitContext(Context, normal, V, L);

    float diff = max(dot(normal, L), 0.0);
    float3 diffuse = diff * base_color.xyz;

    float4 environment_color = Environment_Texture.Sample(Texture_sampler, normal);
    
    float NoL = Context.NoL;
    float NoV = dot(normal, V);
    float3 specular = pow(max(Context.NoH, 0.0), 32.0) * LightColor.xyz;
    //PSOut.Color = NoV;
    
    PSOut.Color = float4(diffuse + specular, 1.0f);
}