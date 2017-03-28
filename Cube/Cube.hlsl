cbuffer ChangedPerFrame : register(b0)
{
    matrix World;
    float3 LightDir;
}

cbuffer ImmutablePerFrame : register(b1)
{
    float4 LightColor;
}

cbuffer MayChanged : register(b2)
{
    matrix View;
    matrix Projection;
}

struct VS_INPUT
{
    float4 Pos: POSITION;
    float4 Norm: NORMAL;
    float2 UV : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos: SV_POSITION;
    float4 Norm : NORMAL;
    float2 UV: TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    //no scale, simply mul the transform matrix
    output.Norm = mul(input.Norm, World);
    output.UV = input.UV;
    return output;
}

Texture2D MainTexture;
SamplerState Sampler;

float4 PS(PS_INPUT input): SV_Target
{
    float4 finalColor = saturate(dot((float3)LightDir, (float3)input.Norm) * LightColor);
    finalColor.a = 1;
    finalColor += MainTexture.Sample(Sampler, input.UV);
    return finalColor;
}