#include "Basic3D.hlsli"
#include "BasicLighting.hlsli"

cbuffer GlobalLightingInfo : register(b0)
{
    float3 EyePos;
    Light Lights[10];
}

cbuffer PerObjectLightingInfo : register(b1)
{
    Material ObjectMaterial;
}

Texture2D Texture;
SamplerState Sampler;

float4 main(VertexOutput input) : SV_TARGET
{
    float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f), totalSpec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 10; ++i)
    {
        float diffuse, spec;
        ComputeLighting(EyePos, input.PositionWS.xyz, input.NormalWS, Lights[i], ObjectMaterial, diffuse, spec);
        totalDiffuse += diffuse;
        totalSpec += spec;
    }
    totalDiffuse = saturate(totalDiffuse);
    totalSpec = saturate(totalSpec);
    return (totalDiffuse + totalSpec) * Texture.Sample(Sampler, input.TexCoord);
}