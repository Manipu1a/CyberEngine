

static const float PI = 3.14159265358979323846;

#ifndef NUM_PHI_SAMPLES
#define NUM_PHI_SAMPLES 64
#endif

#ifndef NUM_THETA_SAMPLES
#define NUM_THETA_SAMPLES 32
#endif

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
    float3 N = normalize(world_pos);
    float3 up = float3(0.0f, 1.0f, 0.0f); // 假设上方向为Y轴
    float3 right = normalize(cross(up, N));

    const float delta_phi = 2.0 * PI / float(NUM_PHI_SAMPLES);
    const float delta_theta = 0.5 * PI / float(NUM_THETA_SAMPLES);

    float3 color = float3(0.0f, 0.0f, 0.0f);
    float sample_count = 0.0f;
    for (int i = 0; i < NUM_PHI_SAMPLES; ++i)
    {
        float phi = float(i) * delta_phi;
        for (int j = 0; j < NUM_THETA_SAMPLES; ++j)
        {
            float theta = float(j) * delta_theta;
            float3 temp = cos(phi) * right + sin(phi) * up;
            float3 sample_dir = cos(theta) * N + sin(theta) * temp;
            float2 uv_sample = DirectionToSphericalUV(sample_dir);
            color += Environment_Texture.Sample(Texture_sampler, uv_sample).rgb;
            sample_count += 1.0f;
        }
    }

    Color = float4(PI * color / sample_count, 1.0f);
}