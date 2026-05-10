cbuffer SceneConstants : register(b0)
{
    float4x4 view_proj_matrix;
    float4x4 model_matrix;
    float4 camera_pos;
    float4 light_direction;
    float4 light_color;
};

Texture2D BaseColorTexture : register(t0);
SamplerState Texture_sampler : register(s0);

struct PSInput
{
    float4 position  : SV_POSITION;
    float3 world_pos : WORLD_POS;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 base_color = BaseColorTexture.Sample(Texture_sampler, input.uv);

    // Simple directional lighting
    float3 N = normalize(input.normal);
    float3 L = normalize(-light_direction.xyz);
    float NdotL = max(dot(N, L), 0.0);

    float3 ambient = 0.15;
    float3 diffuse = light_color.rgb * NdotL;
    float3 lighting = ambient + diffuse;

    float3 final_color = base_color.rgb * lighting;
    return float4(final_color, base_color.a);
}
