#pragma once

#include "Shaders.hpp"
#include "CBStructs.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Transformation.hpp"

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

        struct alignas(16) BasicCb
        {
            DirectX::XMMATRIX WorldViewProj;
            DirectX::XMMATRIX World;
            DirectX::XMMATRIX WorldInvTranspose;
        };
    }


    class Predefined
    {
    public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        Predefined(ID3D11Device&);
        ~Predefined();

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

    private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeBasicVS(ID3D11Device&);
        void MakeBasicPS(ID3D11Device&);
        void MakeStencilStates(ID3D11Device&);
        void MakeBlendingStates(ID3D11Device&);
        void MakeRasterizerStates(ID3D11Device&);

        //Default white texture
        wrl::ComPtr<ID3D11ShaderResourceView> white_;

        wrl::ComPtr<ID3D11SamplerState> defaultSampler_;
        wrl::ComPtr<ID3D11SamplerState> repeatSampler_;

        //default shaders
        VertexShader basicVS_;
        PixelShader basicPS_;

        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_, drawnOnly_, noDoubleBlending_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_, transparent_;
        wrl::ComPtr<ID3D11RasterizerState> cullClockWise_, wireFrameOnly_;
    };

    struct BasicRenderable
    {
        BasicRenderable() = default;

        template<typename VertexT, std::size_t VN, std::size_t IN_>
        BasicRenderable(ID3D11Device& device,
            const Predefined& predefined,
            const VertexT(&vertices)[VN],
            const std::uint16_t (&indices)[IN_],
            wrl::ComPtr<ID3D11ShaderResourceView> texture,
            wrl::ComPtr<ID3D11SamplerState> sampler)
            : BasicRenderable{ device, predefined, CpuMeshView<VertexT>{gsl::make_span(vertices), gsl::make_span(indices)}, std::move(texture), std::move(sampler) }
        {}

        template<typename VertexT>
        BasicRenderable(ID3D11Device& device,
            const Predefined& predefined,
            CpuMeshView<VertexT> cpuMesh,
            wrl::ComPtr<ID3D11ShaderResourceView> texture,
            wrl::ComPtr<ID3D11SamplerState> sampler)
            : Mesh{ device, cpuMesh },
            VS{ predefined.GetBasicVS() },
            PS{ predefined.GetBasicPS() },
            Texture{ texture ? texture : predefined.GetWhite() },
            Sampler {sampler ? sampler : predefined.GetDefaultSampler()},
            BasicCb{ MakeConstantBuffer<cb::BasicCb>(device) },
            GlobalLightingCb{ MakeConstantBuffer<cb::GlobalLightingInfo>(device) },
            PerObjectCb{ MakeConstantBuffer<cb::PerObjectLightingInfo>(device) }
        {}

        GpuMesh Mesh;
        VertexShader VS;
        PixelShader PS;
        wrl::ComPtr<ID3D11ShaderResourceView> Texture;
        wrl::ComPtr<ID3D11SamplerState> Sampler;
        ConstantBuffer<cb::BasicCb> BasicCb;
        ConstantBuffer<cb::GlobalLightingInfo> GlobalLightingCb;
        ConstantBuffer<cb::PerObjectLightingInfo> PerObjectCb;
    };

    struct BasicObject
    {
        BasicObject() = default;
        BasicObject(BasicRenderable renderable,
            Smoothness material,
            DirectX::XMMATRIX matrix);

        DirectX::XMMATRIX GetWorld() const noexcept
        {
            return DirectX::XMLoadFloat4x4(&Transform);
        }

        BasicRenderable Renderable;
        Smoothness Material;
        DirectX::XMFLOAT4X4 Transform;
    };

    struct BasicDrawContext
    {
        ID3D11DeviceContext& Context;
        const Camera& Camera;
        gsl::span<dx::Light> Lights;
    };

    void UpdateAndDraw(const BasicDrawContext& drawContext, const BasicObject& object);
    void DrawBasic(ID3D11DeviceContext&, const BasicRenderable&);


}