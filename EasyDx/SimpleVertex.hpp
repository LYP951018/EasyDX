#pragma once

#include "DXDef.hpp"
#include <DirectXMath.h>

namespace dx
{
    struct SimpleVertex
    {
        DirectX::XMFLOAT3 Pos, Normal, TangentU;
        DirectX::XMFLOAT2 TexCoord;

        static gsl::span<const D3D11_INPUT_ELEMENT_DESC> GetDesc() noexcept;
    };

    template<>
    struct is_vertex<SimpleVertex> : std::true_type {};

    //TODO: is this a vertex? 
    struct alignas(16) InstancedData
    {
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX InvTransWorld;
    };

    gsl::span<const D3D11_INPUT_ELEMENT_DESC> GetInstancedLayout() noexcept;

}