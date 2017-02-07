cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 LightDir;
    float4 LightColor;
    float4 OutputColor;
}

struct VS_INPUT
{
    float4 Pos: POSITION;
    float4 Norm: NORMAL;
};

struct PS_INPUT
{
    float4 Pos: SV_POSITION;
    float4 Norm: TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    //no scale, simply mul the transform matrix
    output.Norm = mul(input.Norm, World);
    return output;
}

float4 PS(PS_INPUT input): SV_Target
{
    float4 finalColor = saturate(dot((float3)LightDir, input.Norm) * LightColor);
    finalColor.a = 1;
    return finalColor;
}