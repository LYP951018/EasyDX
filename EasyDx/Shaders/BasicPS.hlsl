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
    float4 diffuse, spec;
    ComputeLighting(EyePos, input.PositionWS.xyz, input.NormalWS, /*FIXME*/Lights[0], ObjectMaterial, diffuse, spec);
    return (diffuse + spec) * Texture.Sample(Sampler, input.TexCoord);
}