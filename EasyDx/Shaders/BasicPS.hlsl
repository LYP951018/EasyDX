#include "Basic3D.hlsli"
#include "BasicLighting.hlsli"

cbuffer GlobalLightingInfo : register(b0)
{
    float3 dx_EyePos;
    int dx_LightCount;
    Light dx_Lights[10];
	Material ObjectMaterial;

}

//cbuffer PerObjectLightingInfo : register(b1)
//{
//}

Texture2D Texture;
SamplerState Sampler;

float4 main(dx::Outputs::PosNormalTex input) : SV_TARGET
{
    float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f), totalSpec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < dx_LightCount; ++i)
    {
        float4 diffuse = 0.0f, spec = 0.0f;
        ComputeLighting(dx_EyePos, input.PositionWS.xyz, input.NormalWS, dx_Lights[i], ObjectMaterial, diffuse, spec);
        totalDiffuse += diffuse;
        totalSpec += spec;
    }
    totalDiffuse = saturate(totalDiffuse);
    totalSpec = saturate(totalSpec);
    float4 total = totalDiffuse + totalSpec;
    total.w = 1.0f;
    if (ObjectMaterial.UseTexture)
    {
        total *= Texture.Sample(Sampler, input.TexCoord);
    }
    return total;
}