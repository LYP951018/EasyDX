#include <Basic3D.hlsli>

struct InstancedVertexInput
{
    float4 Position : POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;

    matrix WorldMatrix : WORLDMATRIX;
    matrix InvTransWorldMatrix : INVTRANSWORLDMATRIX;
};

cbuffer TfMatrices : register(b0)
{
    matrix ViewProj;
}

dx::Outputs::PosNormalTex main(InstancedVertexInput input)
{
    dx::Outputs::PosNormalTex output;
    input.Position.w = 1.0f;
    input.Normal.w = 0.0f;
    output.PositionWS = float4(mul(input.WorldMatrix, input.Position).xyz, 1.0f);
    output.Position = mul(ViewProj, output.PositionWS);
    output.NormalWS = float4(mul(input.InvTransWorldMatrix, input.Normal).xyz, 0.0f);
    output.TexCoord = float2(0.0f, 0.0f);
    return output;
}