#pragma once

#include "Resources/Shaders.hpp"
#include "CBStructs.hpp"
#include "Material.hpp"
#include "Light.hpp"
#include "Resources/Buffers.hpp"
#include "Resources/InputLayout.hpp"
#include "Vertex.hpp"
#include "Object.hpp"
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

        wrl::ComPtr<ID3D11ShaderResourceView> GetWhite() const { return white_; }

        VertexShader GetBasicVS() const { return basicVS_; }
        PixelShader GetBasicPS() const { return basicPS_; }

        wrl::ComPtr<ID3D11DepthStencilState> GetStencilAlways() const;
        wrl::ComPtr<ID3D11DepthStencilState> GetDrawnOnly() const;
        wrl::ComPtr<ID3D11DepthStencilState> GetNoDoubleBlending() const;

        wrl::ComPtr<ID3D11BlendState> GetNoWriteToRT() const;
        wrl::ComPtr<ID3D11BlendState> GetTransparent() const;

        ID3D11RasterizerState* GetCullClockwise() const { return cullClockWise_.Get(); }
        ID3D11RasterizerState* GetWireFrameOnly() const { return wireFrameOnly_.Get(); }

        wrl::ComPtr<ID3D11SamplerState> GetDefaultSampler() const;
        wrl::ComPtr<ID3D11SamplerState> GetRepeatSampler() const;

        InputLayoutAllocator& InputLayouts() { return m_globalInputLayoutAllocator; }
        const InputLayoutAllocator& InputLayouts() const { return m_globalInputLayoutAllocator; }

      private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeStencilStates(ID3D11Device&);
        void MakeBlendingStates(ID3D11Device&);
        void MakeRasterizerStates(ID3D11Device&);
        void MakeLayouts(ID3D11Device& device);

        // Default white texture
        wrl::ComPtr<ID3D11ShaderResourceView> white_;

        wrl::ComPtr<ID3D11SamplerState> defaultSampler_;
        wrl::ComPtr<ID3D11SamplerState> repeatSampler_;

        std::unordered_map<std::string, VertexShader> m_vertexShaders;
        std::unordered_map<std::string, PixelShader> m_pixelShaders;

        InputLayoutAllocator m_globalInputLayoutAllocator;
        VertexShader basicVS_;
        PixelShader basicPS_;
        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_, drawnOnly_, noDoubleBlending_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_, transparent_;
        wrl::ComPtr<ID3D11RasterizerState> cullClockWise_, wireFrameOnly_;
    };

    std::shared_ptr<Material>
    MakeBasicLightingMaterial(const PredefinedResources& predefined,
                              wrl::ComPtr<ID3D11ShaderResourceView> mainTexture,
                              const dx::Smoothness& smoothness);

    std::unique_ptr<dx::Object>
    MakeObjectWithDefaultRendering(ID3D11Device& device3D, const PredefinedResources& predefined, gsl::span<const dx::PositionType> positions,
               gsl::span<const dx::VectorType> normals, gsl::span<const dx::TexCoordType> texCoords,
               gsl::span<const dx::ShortIndex> indices,
               wrl::ComPtr<ID3D11ShaderResourceView> mainTexture, const dx::Smoothness& smoothness);
} // namespace dx