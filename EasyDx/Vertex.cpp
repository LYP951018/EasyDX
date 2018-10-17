#include "pch.hpp"
#include "Vertex.hpp"

namespace dx
{
    PositionType MakePosition(float x, float y, float z) { return PositionType{x, y, z}; }

    VectorType MakeDir(float x, float y, float z, float w) { return VectorType{x, y, z}; }

    TexCoordType MakeTexCoord(float x, float y) { return TexCoordType{x, y}; }

    ColorType MakeColor(float r, float g, float b, float a) { return ColorType{r, g, b, a}; }

} // namespace dx
