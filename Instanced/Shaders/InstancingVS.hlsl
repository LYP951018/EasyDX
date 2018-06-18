#include <Basic3D.hlsli>

cbuffer TfMatrices : register(b0)
{
    matrix ViewProj;
}

dx::VertexOutput main(dx::InstancedVertexInput input)
{
    dx::VertexOutput output;
    output.PositionWS = float4(mul(input.WorldMatrix, input.Position).xyz, 1.0f);
    output.Position = mul(ViewProj, output.PositionWS);
    output.NormalWS = float4(mul(input.InvTransWorldMatrix, input.Normal).xyz, 0.0f);
    output.TexCoord = float2(0.0f, 0.0f);
    return output;
}