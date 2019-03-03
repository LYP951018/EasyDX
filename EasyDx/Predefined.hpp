#pragma once

#include "CBStructs.hpp"
#include "Material.hpp"
#include "Light.hpp"
#include "Resources/Buffers.hpp"
#include "Resources/InputLayout.hpp"
#include "Vertex.hpp"
#include "Object.hpp"
#include "Transform.hpp"
#include <d3d11.h>

namespace dx
{
    class Camera;

    namespace cb
    {
        struct alignas(16) GlobalLightingInfo
        {
            DirectX::XMFLOAT3 EyePos;
            int LightCount;
            Light Lights[10];
        };

        struct alignas(16) PerObjectLightingInfo
        {
            Material Smoothness;
        };

        struct alignas(16) MvpCb
        {
            DirectX::XMMATRIX WorldViewProj;
            DirectX::XMMATRIX World;
            DirectX::XMMATRIX WorldInvTranspose;
        };
    } // namespace cb

    class PredefinedResources
    {
      public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        PredefinedResources(ID3D11Device&);
        ~PredefinedResources();

        static wrl::ComPtr<ID3D11ShaderResourceView> GetWhite();

        static wrl::ComPtr<ID3D11DepthStencilState> GetStencilAlways();
        static wrl::ComPtr<ID3D11DepthStencilState> GetDrawnOnly();
        static wrl::ComPtr<ID3D11DepthStencilState> GetNoDoubleBlending();

        static wrl::ComPtr<ID3D11BlendState> GetNoWriteToRT();
        static wrl::ComPtr<ID3D11BlendState> GetTransparent();

        static ID3D11RasterizerState* GetCullClockwise();
        static ID3D11RasterizerState* GetWireFrameOnly();

        static wrl::ComPtr<ID3D11SamplerState> GetDefaultSampler();
        static wrl::ComPtr<ID3D11SamplerState> GetRepeatSampler();
        static wrl::ComPtr<ID3D11SamplerState> GetShadowMapSampler();

        static void Setup(ID3D11Device& device3D);
        static const PredefinedResources& GetInstance();

      private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeStencilStates(ID3D11Device&);
        void MakeBlendingStates(ID3D11Device&);
        void MakeRasterizerStates(ID3D11Device&);

        // Default white texture
        wrl::ComPtr<ID3D11ShaderResourceView> white_;

        wrl::ComPtr<ID3D11SamplerState> defaultSampler_, repeatSampler_, m_shadowMapSampler;

        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_, drawnOnly_, noDoubleBlending_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_, transparent_;
        wrl::ComPtr<ID3D11RasterizerState> cullClockWise_, wireFrameOnly_;
    };

    void PresetupBasicPsCb(ShaderInputs& psInputs, const PredefinedResources& predefined, const dx::Smoothness& smoothness,
                           wrl::ComPtr<ID3D11ShaderResourceView> mainTexture = {},
                           wrl::ComPtr<ID3D11SamplerState> sampler = {});

    std::shared_ptr<Material>
    MakeBasicLightingMaterial(const PredefinedResources& predefined,
                              const dx::Smoothness& smoothness,
                              wrl::ComPtr<ID3D11ShaderResourceView> mainTexture = {}, 
                                wrl::ComPtr<ID3D11SamplerState> sampler = {});
} // namespace dx