#pragma once

#include "Common.hpp"
#include <DirectXMath.h>
#include <utility>

namespace dx
{
    struct Smoothness
    {
        DirectX::XMFLOAT4 Amibient, Diffuse, Specular, Reflect;
        float SpecularPower;
    };

    struct Material
    {
        Smoothness Smooth;
        DirectX::XMFLOAT4 Color;
        std::pair<wrl::ComPtr<ID3D11ShaderResourceView>,
            wrl::ComPtr<ID3D11SamplerState>> MainTexture;
    };
}