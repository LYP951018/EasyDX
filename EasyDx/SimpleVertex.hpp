#pragma once

#include "DXDef.hpp"
#include <DirectXMath.h>
#include <gsl/span>

namespace dx
{
    struct SimpleVertex
    {
        DirectX::XMFLOAT3 Pos, Normal;
        DirectX::XMFLOAT2 TexCoord;

        static gsl::span<const D3D11_INPUT_ELEMENT_DESC> GetLayout() noexcept;
    };
}