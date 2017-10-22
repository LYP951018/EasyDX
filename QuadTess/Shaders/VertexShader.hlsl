#include "Common.hlsli"

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.Position = input.Position;
    output.Position.w = 1.0f;
    return output;
}