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

    using BasicLightingPixelShader = PixelShader<cb::GlobalLightingInfo, cb::PerObjectLightingInfo>;
    using BasicLightingVertexShader = VertexShader<SimpleVertex, cb::BasicCb>;

    class PredefinedResources
    {
    public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        PredefinedResources(ID3D11Device&);
        ~PredefinedResources();

        wrl::ComPtr<ID3D11ShaderResourceView> GetWhite() const { return white_; }

        BasicLightingVertexShader GetBasicVS() const { return basicVS_; }
        BasicLightingPixelShader GetBasicPS() const { return basicPS_; }

        wrl::ComPtr<ID3D11DepthStencilState> GetStencilAlways() const;
        wrl::ComPtr<ID3D11DepthStencilState> GetDrawnOnly() const;
        wrl::ComPtr<ID3D11DepthStencilState> GetNoDoubleBlending() const;

        wrl::ComPtr<ID3D11BlendState> GetNoWriteToRT() const;
        wrl::ComPtr<ID3D11BlendState> GetTransparent() const;

        ID3D11RasterizerState* GetCullClockwise() const { return cullClockWise_.Get(); }
        ID3D11RasterizerState* GetWireFrameOnly() const { return wireFrameOnly_.Get(); }

        wrl::ComPtr<ID3D11SamplerState> GetDefaultSampler() const;
        wrl::ComPtr<ID3D11SamplerState> GetRepeatSampler() const;

        wrl::ComPtr<ID3D11InputLayout> GetSimpleLayout() const { return simpleLayout_; }

    private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeStencilStates(ID3D11Device&);
        void MakeBlendingStates(ID3D11Device&);
        void MakeRasterizerStates(ID3D11Device&);
        void MakeLayouts(ID3D11Device& device);

        //Default white texture
        wrl::ComPtr<ID3D11ShaderResourceView> white_;

        wrl::ComPtr<ID3D11SamplerState> defaultSampler_;
        wrl::ComPtr<ID3D11SamplerState> repeatSampler_;

        //default shaders
        BasicLightingVertexShader basicVS_;
        BasicLightingPixelShader basicPS_;

        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_, drawnOnly_, noDoubleBlending_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_, transparent_;
        wrl::ComPtr<ID3D11RasterizerState> cullClockWise_, wireFrameOnly_;

        wrl::ComPtr<ID3D11InputLayout> simpleLayout_;
    };

    struct BasicDrawContext;

    void SetupBasicLighting(const BasicDrawContext& drawContext,
        BasicLightingPixelShader& ps,
        const Smoothness& smoothness,
        ID3D11ShaderResourceView* tex = nullptr,
        ID3D11SamplerState* sampler = nullptr);


    //TODO: rename to BasicPipelineObjects ?
    struct BasicRenderable
    {
        wrl::ComPtr<ID3D11InputLayout> InputLayout;
        GpuMesh Mesh;
        BasicLightingVertexShader VS;
        BasicLightingPixelShader PS;
        wrl::ComPtr<ID3D11ShaderResourceView> Texture;
        wrl::ComPtr<ID3D11SamplerState> Sampler;
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