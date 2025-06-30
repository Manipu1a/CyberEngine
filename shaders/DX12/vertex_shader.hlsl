struct VSInput
{
    float3 pos : ATTRIB0;
    float2 uv  : ATTRIB1;
    float4 col : ATTRIB2;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

void VSMain(in VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos = float4(VSIn.pos, 1.0f);
    PSIn.Color = VSIn.col;
    PSIn.UV = VSIn.uv;
}