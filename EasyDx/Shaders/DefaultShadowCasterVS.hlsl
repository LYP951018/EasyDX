#include "Basic3D.hlsli"

cbuffer Cb : register(b0)
{
	matrix dx_WorldViewProjMatrix;
};

float4 main(dx::Inputs::Pos input) : SV_Position
{
	input.Position.w = 1.0f;
    return mul(dx_WorldViewProjMatrix, input.Position);
}