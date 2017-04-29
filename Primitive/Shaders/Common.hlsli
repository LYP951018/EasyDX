#include <BasicLighting.hlsli>

cbuffer cbPerObject : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    matrix WorldInvTranspose;
    Material MainMaterial;
}

cbuffer cbPerFrame : register(b1)
{
    float3 EyePos;
    Light Dlight;
}