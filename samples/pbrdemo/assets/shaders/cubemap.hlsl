
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS;  // 添加这一行
};

cbuffer ConstantTransform : register(b0)
{
    float4x4 rotation;
};

void VSMain(in uint VertexID: SV_VertexID,
            out VSOutput output)
{
    float2 PoxXY[4];
    PoxXY[0] = float2(-1.0f,  -1.0f);
    PoxXY[1] = float2(-1.0f,  1.0f);
    PoxXY[2] = float2(1.0f, -1.0f);
    PoxXY[3] = float2( 1.0f, 1.0f);

    output.Pos = float4(PoxXY[VertexID], 1.0f, 1.0f);
    float4 world = mul(rotation, output.Pos);
    output.ClipPos = float4(world.xyz / world.w, 1.0f);
}