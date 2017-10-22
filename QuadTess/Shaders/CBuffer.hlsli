cbuffer cbPerObject : register(b0)
{
    float4x4 World;
    float4x4 WorldViewProj;
};

cbuffer cbPerFrame : register(b1)
{
    float3 EyePos;
};