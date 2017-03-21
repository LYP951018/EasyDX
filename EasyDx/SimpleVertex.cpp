#include "SimpleVertex.hpp"
#include <d3d11.h>

namespace dx
{
    constexpr D3D11_INPUT_ELEMENT_DESC SimpleVertexLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    gsl::span<const D3D11_INPUT_ELEMENT_DESC> SimpleVertex::GetLayout() noexcept
    {
        return gsl::make_span(SimpleVertexLayout);
    }
}
