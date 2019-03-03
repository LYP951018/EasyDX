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

    inline constexpr Smoothness kDefaultSmoothness =
        Smoothness{DirectX::XMFLOAT4{0, 0, 0, 1.0}, DirectX::XMFLOAT4{0.55, 0.55, 0.55, 1.0},
                   DirectX::XMFLOAT4{0.7, 0.7, 0.7, 1.0}, DirectX::XMFLOAT4{}, float{32.0f}};

    struct BlendSettings
    {
        wrl::ComPtr<ID3D11BlendState> BlendState;
        std::array<float, 4> BlendFactor{1.0f, 1.0f, 1.0f, 1.0f};
        std::uint32_t SampleMask;
    };

    void SetupBlending(ID3D11DeviceContext& context3D, const BlendSettings& blendSettings);

    struct DepthStencilSettings
    {
        wrl::ComPtr<ID3D11DepthStencilState> StencilState;
        std::uint32_t StencilRef;
    };

    void SetupDepthStencilStates(ID3D11DeviceContext& context3D,
                                 const DepthStencilSettings& depthStencilSettings);

    struct Pass
    {
        ShaderCollection Shaders;
        BlendSettings Blending;
        DepthStencilSettings DepthStencil;
        wrl::ComPtr<ID3D11RasterizerState> RasterizerState;
        std::string_view Name;
    };

    struct PassWithShaderInputs
    {
        std::shared_ptr<Pass> pass;
        ShaderInputs inputs;
    };

    class PredefinedPasses
    {
      public:
        static void Initialize();
        static std::shared_ptr<Pass> GetPlainShadowCaster();
    };

    void SetupRasterizerState(ID3D11DeviceContext& context3D, ID3D11RasterizerState& rasterState);
    void SetupPass(ID3D11DeviceContext& context3D, const Pass& pass);

    // TODO: multi pass?
    struct Material
    {
        Material(std::shared_ptr<Pass> mainPass, std::shared_ptr<Pass> shadowCasterPass)
            : mainPass{std::move(mainPass)}, shadowCasterPass{std::move(shadowCasterPass)}
        {}

        Material(std::shared_ptr<Pass> mainPass)
            : Material{std::move(mainPass), PredefinedPasses::GetPlainShadowCaster()}
        {}

        DEFAULT_MOVE(Material)

        PassWithShaderInputs mainPass;
        PassWithShaderInputs shadowCasterPass;
    };
} // namespace dx