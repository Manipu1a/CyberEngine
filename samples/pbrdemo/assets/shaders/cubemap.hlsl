
cbuffer ConstantTransform : register(b0)
{
    float4x4 rotation;
};

void VSMain(in uint VertexID: SV_VertexID,
            out float4 position: SV_Position,
            out float3 world_pos : TEXCOORD0)
{
    float2 PoxXY[4];
    PoxXY[0] = float2(-1.0f,  -1.0f);
    PoxXY[1] = float2(-1.0f,  1.0f);
    PoxXY[2] = float2(1.0f, -1.0f);
    PoxXY[3] = float2( 1.0f, 1.0f);

    position = float4(PoxXY[VertexID], 1.0f, 1.0f);
    float4 world = mul(rotation, position);
    world_pos = world.xyz / world.w;
}