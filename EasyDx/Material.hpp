#pragma once

#include <DirectXMath.h>
#include "Resources/Shaders.hpp"

namespace dx
{
    struct ShaderCollection;

    struct Smoothness
    {
        DirectX::XMFLOAT4 Amibient, Diffuse, Specular, Emissive;
        float SpecularPower;
    };

    struct BlendSettings
    {
        wrl::ComPtr<ID3D11BlendState> BlendState;
        std::array<float, 4> BlendFactor{1.0f, 1.0f, 1.0f, 1.0f};
        std::uint32_t SampleMask;
    };

    struct DepthStencilSettings
    {
        wrl::ComPtr<ID3D11DepthStencilState> StencilState;
        std::uint32_t StencilRef;
    };

    struct Pass
    {
        ShaderCollection Shaders;
        BlendSettings Blending;
        DepthStencilSettings DepthStencil;
        wrl::ComPtr<ID3D11RasterizerState> RasterizerState;
    };

    struct Material
    {
        Material(std::vector<Pass> passes)
            : Passes{std::move(passes)}
        {}

        DEFAULT_MOVE(Material)

        std::vector<Pass> Passes;
    };
} // namespace dx