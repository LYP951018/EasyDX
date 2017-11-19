#include "Common.hlsli"
#include <Basic3D.hlsli>
#include "CBuffer.hlsli"

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;
    return float4(
        pow(invT, 3.0f),
        3.0f * t * invT * invT,
        3 * t * t * invT,
        pow(t, 3.0)
    );
}

float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    return float4(
        -3 * invT * invT,
        3 * invT * invT - 6 * t * invT,
        6 * t * invT - 3 * t * t,
        3 * t * t
    );
}

float3 SumUp(int line_, float4 basis, const OutputPatch<HullControlPointOutput, PATCH_COUNT> patch)
{
    int base = line_ * 4;
    return basis.x * patch[base].Position
        + basis.y * patch[base + 1].Position
        + basis.z * patch[base + 2].Position
        + basis.w * patch[base + 3].Position;
}

float3 EvalBezier(const OutputPatch<HullControlPointOutput, PATCH_COUNT> patch,
    float4 basisU,
    float4 basisV)
{
    return SumUp(0, basisU, patch) * basisV.x
        + SumUp(1, basisU, patch) * basisV.y
        + SumUp(2, basisU, patch) * basisV.z
        + SumUp(3, basisU, patch) * basisV.w;
}


[domain("quad")]
dx::VertexOutput main(
    ConstantHullOutput input,
    float2 domain : SV_DomainLocation,
    const OutputPatch<HullControlPointOutput, PATCH_COUNT> patch)
{
    float4 basisU = BernsteinBasis(domain.x);
    float4 basisV = BernsteinBasis(domain.y);
    
    float4 dBasisU = dBernsteinBasis(domain.x);
    float4 dBasisV = dBernsteinBasis(domain.y);

    float3 worldPos = EvalBezier(patch, basisU, basisV);
    float3 tangent = EvalBezier(patch, dBasisU, basisV);
    float3 biTangent = EvalBezier(patch, basisU, dBasisV);

    float3 normal = normalize(cross(tangent, biTangent));
    dx::VertexOutput output;

    output.Position = mul(WorldViewProj, float4(worldPos, 1.0f));
    output.PositionWS = float4(worldPos, 1.0f);
    output.NormalWS = normal;
    output.TexCoord = 0.0f;
    return output;
}
