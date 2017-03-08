#pragma once

#include <DirectXMath.h>

namespace dx
{
    struct SimpleVertex
    {
        DirectX::XMFLOAT3 Pos, Normal;
        DirectX::XMFLOAT2 TexCoord;
    };
}