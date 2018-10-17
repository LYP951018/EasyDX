#include "Basic3D.hlsli"

cbuffer TransformMatrices : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    matrix WorldInvTranspose;
}

dx::Outputs::PosNormalTex main(dx::Inputs::PosNormalTex input)
{
    dx::Outputs::PosNormalTex output;
    input.Position.w = 0.0f;
    input.Normal.w = 1.0f;
    output.PositionWS = mul(World, input.Position);
    output.Position = mul(WorldViewProj, input.Position);
    output.NormalWS = mul(WorldInvTranspose, input.Normal).xyz;
    output.TexCoord = input.TexCoord;
    return output;
}