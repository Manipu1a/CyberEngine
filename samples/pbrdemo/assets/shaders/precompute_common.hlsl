static const float PI = 3.14159265358979323846;

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

// Returns a random cosine-weighted direction on the hemisphere around z = 1.
void SampleDirectionCosineHemisphere(in  float2 UV,  // Normal random variables
                                     out float3 Dir, // Direction
                                     out float  Prob // Probability of the generated direction
                                     )
{
    Dir.x = cos(2.0 * PI * UV.x) * sqrt(1.0 - UV.y);
    Dir.y = sin(2.0 * PI * UV.x) * sqrt(1.0 - UV.y);
    Dir.z = sqrt(UV.y);

    // Avoid zero probability
    Prob = max(Dir.z, 1e-6) / PI;
}

// Returns a random cosine-weighted direction on the hemisphere around N.
void SampleDirectionCosineHemisphere(in  float3 N,   // Normal
                                     in  float2 UV,  // Normal random variables
                                     out float3 Dir, // Direction
                                     out float  Prob // Probability of the generated direction
                                     )
{
    float3 T = normalize(cross(N, abs(N.y) > 0.5 ? float3(1.0, 0.0, 0.0) : float3(0.0, 1.0, 0.0)));
    float3 B = cross(T, N);
    SampleDirectionCosineHemisphere(UV, Dir, Prob);
    Dir = normalize(Dir.x * T + Dir.y * B + Dir.z * N);
}

