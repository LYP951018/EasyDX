#pragma once

#include "Common.hpp"
#include <DirectXMath.h>
#include <utility>

namespace dx
{
    struct Smoothness
    {
        DirectX::XMFLOAT3 Amibient, Diffuse, Specular;
        float SpecularPower;
    };

    struct Material
    {
        Smoothness Smooth;
        std::pair<wrl::ComPtr<ID3D11ShaderResourceView>,
            wrl::ComPtr<ID3D11SamplerState>> MainTexture;
    };
}