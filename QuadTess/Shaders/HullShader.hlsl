#include "Common.hlsli"
#include "CBuffer.hlsli"

ConstantHullOutput CalcHSPatchConstants(
	InputPatch<VertexOutput, PATCH_COUNT> patches,
	uint PatchID : SV_PrimitiveID)
{
    ConstantHullOutput Output;

    //float3 center = 0.25f * (patches[0].Position + patches[1].Position + patches[2].Position + patches[3].Position);
    //float4 centerWorld = mul(float4(center, 1.0f), World);
    //float dist = distance(center.xyz, EyePos);
    //const float d0 = 20.0f;
    //const float d1 = 100.0f;
    //float tes = saturate((d1 - dist) / (d1 - d0)) * 64.0f;
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
	uint PatchID : SV_PrimitiveID )
{
    HullControlPointOutput Output;

    Output.Position = ip[i].Position;

	return Output;
}
