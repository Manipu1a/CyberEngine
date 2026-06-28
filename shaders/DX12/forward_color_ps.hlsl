cbuffer ForwardSceneConstants : register(b0)
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
    float3 normal = normalize(input.normal);
    float3 light = normalize(-light_direction.xyz);
    float ndotl = max(dot(normal, light), 0.0);
    float3 ambient = 0.15 * base_color.rgb;
    float3 diffuse = light_color.rgb * ndotl * base_color.rgb;
    return float4(ambient + diffuse, base_color.a);
}
