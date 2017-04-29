#include <Basic3D.hlsli>
#include "Common.hlsli"


float4 main(VertexOutput output) : SV_TARGET
{
    float4 diffuse, spec;
    ComputeLighting(EyePos, output.PositionWS.xyz, output.NormalWS, Dlight, MainMaterial, diffuse, spec);
    return diffuse + spec;
}