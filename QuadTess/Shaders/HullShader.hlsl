#include "Common.hlsli"
#include "CBuffer.hlsli"

ConstantHullOutput CalcHSPatchConstants(
    InputPatch<VertexOutput, PATCH_COUNT> patches,
    uint PatchID : SV_PrimitiveID)
{
    ConstantHullOutput Output;

    const int n = 10;
    Output.EdgeTessFactor[0] = n;
    Output.EdgeTessFactor[1] = n;
    Output.EdgeTessFactor[2] = n;
    Output.EdgeTessFactor[3] = n;
    Output.InsideTessFactor[0] = Output.InsideTessFactor[1] = n;

    return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(PATCH_COUNT)]
[patchconstantfunc("CalcHSPatchConstants")]
HullControlPointOutput main(
    InputPatch<VertexOutput, PATCH_COUNT> ip,
    uint i : SV_OutputControlPointID,
    uint PatchID : SV_PrimitiveID)
{
    HullControlPointOutput Output;

    Output.Position = ip[i].Position;

    return Output;
}
