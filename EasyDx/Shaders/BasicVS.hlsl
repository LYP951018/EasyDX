#include "Basic3D.hlsli"

cbuffer TransformMatrices : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    matrix WorldInvTranspose;
}

dx::VertexOutput main(dx::VertexInput input)
{
    dx::VertexOutput output;
    output.PositionWS = mul(World, input.Position);
    output.Position = mul(WorldViewProj, input.Position);
    output.NormalWS = mul(WorldInvTranspose, input.Normal).xyz;
    output.TexCoord = input.TexCoord;
    return output;
}