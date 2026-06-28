cbuffer ForwardSceneConstants : register(b0)
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
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 world_pos = mul(float4(input.position, 1.0), model_matrix);
    output.position = mul(world_pos, view_proj_matrix);
    return output;
}
