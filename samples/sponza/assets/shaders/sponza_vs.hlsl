cbuffer SceneConstants : register(b0)
{
    float4x4 view_proj_matrix;
    float4x4 model_matrix;
    float4 camera_pos;
    float4 light_direction;
    float4 light_color;
};

struct VSInput
{
    float3 position : ATTRIB0;
    float3 normal   : ATTRIB1;
    float2 uv       : ATTRIB2;
};

struct VSOutput
{
    float4 position  : SV_POSITION;
    float3 world_pos : WORLD_POS;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 world_pos = mul(float4(input.position, 1.0), model_matrix);
    output.world_pos = world_pos.xyz;
    output.position = mul(world_pos, view_proj_matrix);
    output.normal = mul(float4(input.normal, 0.0), model_matrix).xyz;
    output.uv = input.uv;
    return output;
}
