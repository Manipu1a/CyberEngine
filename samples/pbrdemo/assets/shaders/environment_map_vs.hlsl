struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS;  // 添加这一行
};

void VSMain(in uint VertexID : SV_VertexID, out VSOutput output)
{
    float2 PosXY[3];
    PosXY[0] = float2(-1.0, -1.0);
    PosXY[1] = float2(-1.0, 3.0);
    PosXY[2] = float2(3.0, -1.0);

    float2 pos = PosXY[VertexID];
    output.Pos = float4(pos, 1.0, 1.0);
    output.ClipPos = output.Pos;
}