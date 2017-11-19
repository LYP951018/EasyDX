#include "Common.hlsli"

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    return output;
}