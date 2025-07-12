

static const float PI = 3.14159265358979323846;

float2 DirectionToSphericalUV(float3 direction)
{
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI;
    return float2(u, v);
}

Texture2D Environment_Texture;
SamplerState Texture_sampler;

void PSMain(in float4 position: SV_Position,
            in float3 world_pos: TEXCOORD0, out float4 Color : SV_Target)
{
    float3 CameraPosition = float3(0.0f, 0.0f, 0.0f); // 假设相机位置为原点
    float3 direction = normalize(world_pos - CameraPosition.xyz); // 计算从相机到世界位置的方向
    float2 uv = DirectionToSphericalUV(direction);
    
    // Set the output color
    Color = Environment_Texture.Sample(Texture_sampler, uv); // 将方向映射到纹理坐标
}