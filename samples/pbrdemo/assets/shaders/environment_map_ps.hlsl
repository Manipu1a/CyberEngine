
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 ClipPos : CLIP_POS; // 添加这一行
};

void PSMain(in VSOutput input, out float4 Color : SV_Target)
{
    // This shader is used to render the environment map
    // It does not perform any operations, as the environment map is a full-screen quad
    // The output color is set to white, as the environment map will be sampled in the fragment shader
    // and the color will be determined by the sampled texture.
    
    // Output color is white
    float4 color = float4(1.0, 1.0, 1.0, 1.0);

    // Set the output color
    Color = input.ClipPos;
}