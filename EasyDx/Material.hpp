#pragma once

#include "Common.hpp"
#include <DirectXMath.h>
#include <vector>

namespace dx
{
    struct Smoothness
    {
        DirectX::XMFLOAT4 Amibient, Diffuse, Specular, Emissive;
        float SpecularPower;
    };

    struct Material
    {
        Smoothness Smooth;
        bool UseTexture;
        std::vector<wrl::ComPtr<ID3D11ShaderResourceView>> Textures;
        std::vector<wrl::ComPtr<ID3D11SamplerState>> Samplers;
    };

    void SetupTextures(ID3D11DeviceContext& context, const Material& material);
}