struct VSInput
{
    float3 pos : ATTRIB0;
    float3 normal : ATTRIB1; // Using normal as color for demonstration
    float3 tangent : ATTRIB2;
    float2 uv : ATTRIB3;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;

    float3 Tangent : TEXCOORD1; // Tangent vector
};

cbuffer Constants
{
    float4x4 ModelMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
}

void VSMain(in VSInput VSIn,
          out PSInput PSIn)
{
    float3 model_position = VSIn.pos;
    model_position.x *= -1.0f; // Invert X axis for right-handed coordinate system

    // Transform position to clip space
    float4x4 worldViewProj = mul(ProjectionMatrix, mul(ViewMatrix, ModelMatrix));

    PSIn.Pos = mul(worldViewProj, float4(model_position, 1.0));
    PSIn.Normal = normalize(mul(ModelMatrix, float4(VSIn.normal, 0.0)).xyz);
    float3 Tangent = normalize(mul(ModelMatrix, float4(VSIn.tangent, 0.0)).xyz); // Transform tangent to clip space
    // Calculate bitangent using cross product

    PSIn.Tangent = Tangent;
    PSIn.UV = VSIn.uv;
}