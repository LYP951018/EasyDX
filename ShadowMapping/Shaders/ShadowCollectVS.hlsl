#include <Basic3D.hlsli>


dx::Outputs::PosTex main(dx::Inputs::PosTex input)
{
    dx::Outputs::PosTex output;
    output.Position = input.Position;
    output.Position.w = 1.0f;
    output.TexCoord = input.TexCoord;
    return output;
}