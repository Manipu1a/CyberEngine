

static const float PI = 3.14159265358979323846;
static const uint NUM_SAMPLES = 1024;

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

cbuffer PrefilterConstants
{
    float Roughness;
};

TextureCube Environment_Texture;
SamplerState Texture_sampler;

float RadicalInverse_Vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_Vdc(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // 球面坐标转换
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    float3 up = abs(N.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f); // 选择与法线N最正交的轴作为上方向
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = normalize(cross(N, tangent));
    float3 sample_dir = normalize(tangent * H.x + bitangent * H.y + N * H.z);
    return normalize(sample_dir);
}

void PSMain(in VSOutput PSIn, out float4 Color : SV_Target)
{
    float3 N = normalize(PSIn.ClipPos.xyz);
    float3 R = N;
    float3 V = R; // 假设视线方向与反射方向相同

    const float delta_phi = 2.0 * PI / float(NUM_PHI_SAMPLES);
    const float delta_theta = 0.5 * PI / float(NUM_THETA_SAMPLES);

    float3 color = float3(0.0f, 0.0f, 0.0f);
    float sample_count = 0.0f;
    for(uint i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 Xi = Hammersley(i, NUM_SAMPLES);
        float3 H = ImportanceSampleGGX(Xi, N, Roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V); // 光线方向

        // 采样环境贴图
        float NoL = max(dot(N, L), 0.0f);
        if(NoL > 0.0)
        {
            color += Environment_Texture.Sample(Texture_sampler, L).rgb * NoL;
            sample_count += NoL;
        }
    }

    Color = float4(color / sample_count, 1.0f);
}