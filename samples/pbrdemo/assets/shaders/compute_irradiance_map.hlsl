

static const float PI = 3.14159265358979323846;

#ifndef NUM_PHI_SAMPLES
#define NUM_PHI_SAMPLES 64
#endif

#ifndef NUM_THETA_SAMPLES
#define NUM_THETA_SAMPLES 32
#endif

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS;  // 添加这一行
};


float2 DirectionToSphericalUV(float3 direction)
{
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = phi / (2.0 * PI) + 0.5;
    float v = theta / PI;
    return float2(u, v);
}

TextureCube Environment_Texture;
SamplerState Texture_sampler;

void PSMain(in VSOutput PSIn, out float4 Color : SV_Target)
{
    float3 N = normalize(PSIn.ClipPos.xyz);
    float3 up = float3(0.0f, 1.0f, 0.0f); // 假设上方向为Y轴
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

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
            color += Environment_Texture.Sample(Texture_sampler, sample_dir).rgb * cos(theta) * sin(theta);
            sample_count += 1.0f;
        }
    }

    Color = PI * float4(color / sample_count, 1.0f);
}