
#include "precompute_common.hlsl"

static const uint NUM_SAMPLES = 1024; // 增加采样数量以提高质量

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


float3 ImportanceSampleGGX(float2 Xi, float3 N, float PerceptualRoughness)
{
    float Roughness = PerceptualRoughness * PerceptualRoughness;
    float a2 = Roughness * Roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt( saturate((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y)));
    float sinTheta = sqrt( saturate(1.0 - cosTheta * cosTheta));

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

// GGX NDF
float D_GGX(float NoH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH2 = NoH * NoH;
    
    float nom = a2;
    float denom = NoH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    
    return nom / denom;
}

// 计算预过滤环境贴图的mipmap级别
float ComputeLOD(float NoH, float roughness)
{
    // 基于粗糙度和PDF来计算合适的mipmap级别
    // 这可以减少高频噪声并提高性能
    float D = D_GGX(NoH, roughness);
    float pdf = (D * NoH / (4.0 * NoH)) + 0.0001;
    float saTexel = 4.0 * PI / (6.0 * 512.0 * 512.0); // 假设环境贴图分辨率为512x512
    float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);
    float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
    return mipLevel;
}

void PSMain(in VSOutput PSIn, out float4 Color : SV_Target)
{
    float3 N = normalize(PSIn.ClipPos.xyz);
    float3 R = N;
    float3 V = R; // 假设视线方向与反射方向相同

    float3 prefilteredColor = float3(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    
    float3 sampleColor = Environment_Texture.Sample(Texture_sampler, N).rgb;
    Color = float4(sampleColor, 1.0f);
    
    for(uint i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 Xi = Hammersley(i, NUM_SAMPLES);
        float3 H = ImportanceSampleGGX(Xi, N, Roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V); // 计算光线方向
        
        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        if(NoL > 0.0)
        {
            // 计算采样的mipmap级别
            float D = D_GGX(NoH, Roughness);
            float pdf = (D * NoH / (4.0 * VoH)) + 0.0001;
            
            // 基于PDF和采样数计算合适的mipmap级别
            float resolution = 512.0; // 环境贴图分辨率
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);
            float mipLevel = Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            
            // 使用计算的mipmap级别采样
            float3 sampleColor = Environment_Texture.Sample(Texture_sampler, L).rgb;
            // 累加带权重的颜色
            prefilteredColor += sampleColor * NoL;
            totalWeight += NoL;
        }
    }

    // 避免除零
    if(totalWeight > 0.0)
    {
        prefilteredColor = prefilteredColor / totalWeight;
    }
    
    Color = float4(prefilteredColor, 1.0f);
}