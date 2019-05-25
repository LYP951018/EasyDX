#include <Basic3D.hlsli>

Texture2D DepthMap;
Texture2DArray ShadowMapArray;

SamplerState NearestPointSampler;

cbuffer Cb
{
    matrix InvProj;
    matrix lightSpaceProjs[4];
    //Near 15 20 50
    float4 Intervals;
};

int IndexFromZ(float z)
{
    float4 comp = z.xxxx > Intervals;
    float index = dot(comp, 1.0f.xxxx) - 1.0f;
    return (int)index;
}

float4 main(dx::Outputs::PosTex input) : SV_Target
{
    //restore depth from depth map
    //map ndc to uv space
    float2 uv = input.TexCoord;
    float2 ndc = uv * 2.0f - 1.0f;
    float depth = DepthMap.Sample(NearestPointSampler, uv).r;
    ndc.y = -ndc.y;
    float4 homoPos = float4(ndc, depth, 1.0f);
    float4 viewSpacePosU = mul(InvProj, homoPos);
    float4 viewSpacePos = viewSpacePosU / viewSpacePosU.w;
    int index = IndexFromZ(viewSpacePos.z);
    float4 lightSpacePos = mul(lightSpaceProjs[index], viewSpacePos);
    lightSpacePos /= lightSpacePos.w;
    float2 lightUV = (lightSpacePos.xy + 1.0f.xx) / 2.0f;
    lightUV.y = 1 - lightUV.y;
    float minimalDepth = ShadowMapArray.Sample(NearestPointSampler,
        float3(lightUV, index));
    float color = (float)(abs(lightSpacePos.z - minimalDepth) <= 0.006f);
    return color.xxxx;
}