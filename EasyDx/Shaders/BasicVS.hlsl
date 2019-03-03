#include "Basic3D.hlsli"

cbuffer TransformMatrices : register(b0)
{
    matrix dx_WorldViewProjMatrix;
	matrix dx_WorldMatrix;
    matrix dx_InvTransWorldMatrix;
}

dx::Outputs::PosNormalTex main(dx::Inputs::PosNormalTex input)
{
    dx::Outputs::PosNormalTex output;
    input.Position.w = 1.0f;
    input.Normal.w = 0.0f;
    output.PositionWS = mul(dx_WorldMatrix, input.Position);
    output.Position = mul(dx_WorldViewProjMatrix, input.Position);
    output.NormalWS = mul(dx_InvTransWorldMatrix, input.Normal).xyz;
    output.TexCoord = input.TexCoord;
    return output;
}