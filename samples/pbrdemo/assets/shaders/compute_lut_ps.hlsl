struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float2 UV    : TEXCOORD; 
};
static const float PI = 3.14159265358979323846;
static const uint NUM_SAMPLES = 1024;

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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}  

float2 IntegrateBRDF(float NoV, float roughness)
{
    float3 V;
    V.x = sqrt(1.0 - NoV * NoV);
    V.y = 0.0;
    V.z = NoV;

    float A = 0.0f;
    float B = 0.0f;

    float3 N = float3(0.0f, 0.0f, 1.0f);

    for(uint i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 Xi = Hammersley(i , NUM_SAMPLES);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NoL = max(L.z, 0.0f);
        float NoH = max(H.z, 0.0f);
        float VoH = max(dot(V, H), 0.0f);

        if(NoL > 0.0f)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VoH) / (NoH * NoV + 0.0001f); // Avoid division by zero
            float Fc = pow(1.0f - VoH, 5.0f); // Fresnel term

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(NUM_SAMPLES);
    B /= float(NUM_SAMPLES);

    return float2(A, B);
}

void PSMain(in PSInput PSIn, out float4 Color : SV_Target) 
{
    float2 UV = PSIn.UV;
    float2 BRDF = IntegrateBRDF(UV.x, UV.y);
    Color = float4(BRDF, 0.0f, 1.0f); // 输出颜色
}
