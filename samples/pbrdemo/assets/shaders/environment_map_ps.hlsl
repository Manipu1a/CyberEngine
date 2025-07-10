
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS; // 添加这一行
};

Texture2D Environment_Texture;
SamplerState Texture_sampler;

cbuffer Constant
{
    float4 CameraPosition; // 相机位置
    float4x4 ViewProjectionInv;
};

static const float PI = 3.14159265358979323846;

float2 DirectionToSphericalUV(float3 direction)
{ 
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI;
    return float2(u, v);
}

void PSMain(in VSOutput input, out float4 Color : SV_Target)
{
    float4 world_pos = mul(ViewProjectionInv, input.ClipPos);
    float3 direction = normalize(world_pos.xyz / world_pos.w - CameraPosition.xyz); // 计算从相机到世界位置的方向
    float2 uv = DirectionToSphericalUV(direction);
    
    // Set the output color
    Color = Environment_Texture.Sample(Texture_sampler, uv); // 将方向映射到纹理坐标
}