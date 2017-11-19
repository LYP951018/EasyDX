struct VertexInput
{
    float3 Position : POSITION;
};

struct VertexOutput
{
    float3 Position : POSITION;
};

struct ConstantHullOutput
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

struct HullControlPointOutput
{
    float3 Position : POSITION;
};

struct DomainOutput
{
    float3 Position : SV_POSITION;
    float3 Normal : SV_NORMAL;
};

#define PATCH_COUNT 16