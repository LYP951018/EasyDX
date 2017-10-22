#include "Common.hlsli"
#include "CBuffer.hlsli"

[domain("quad")]
DomainOutput main(
    ConstantHullOutput input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HullControlPointOutput, 4> patch)
{
    float3 v1 = lerp(patch[0].Position, patch[1].Position, domain.x).xyz;
    float3 v2 = lerp(patch[2].Position, patch[3].Position, domain.x).xyz;
    float3 v = lerp(v1, v2, domain.y);
    v.y = 0.3f * (v.z * sin(v.x) + v.x * cos(v.z));
    DomainOutput output;
    output.Position = mul(float4(v, 1.0f), WorldViewProj);
    return output;
}
