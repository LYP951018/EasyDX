#include <Basic3D.hlsli>
#include "Common.hlsli"

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.PositionWS = mul(World, input.Position);
    output.Position = mul(WorldViewProj, input.Position);
    output.NormalWS = mul(WorldInvTranspose, input.Normal).xyz;
    output.TexCoord = input.TexCoord;
    return output;
}