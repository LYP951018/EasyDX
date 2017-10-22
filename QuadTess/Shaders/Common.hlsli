struct VertexInput
{
    float4 Position : POSITION;
};

struct VertexOutput
{
    float4 Position : POSITION;
};

struct ConstantHullOutput
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

struct HullControlPointOutput
{
    float4 Position : POSITION;
};

struct DomainOutput
{
    float4 Position : SV_POSITION;
};