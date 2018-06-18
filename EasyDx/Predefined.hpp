#pragma once

#include "Resources/Shaders.hpp"
#include "CBStructs.hpp"
#include "Material.hpp"
#include "Transformation.hpp"
#include "Light.hpp"
#include "Resources/Buffers.hpp"
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
    }

    using SimpleLightingPS = PixelShader<cb::GlobalLightingInfo, cb::PerObjectLightingInfo>;

    //仅仅将顶点做变换传递给下一个 Shader 的 VertexShader。
    using MvpTransformVS = VertexShader<cb::MvpCb>;

    class PredefinedResources
    {
    public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        static constexpr auto PosNormTexTangDescs = MakeDescArray(std::array{
            MakeVertex(VSSemantics::kPosition, 0),
            MakeVertex(VSSemantics::kNormal, 1),
            MakeVertex(VSSemantics::kTexCoord, 2),
            MakeVertex(VSSemantics::kTangent, 3)
        });

        PredefinedResources(ID3D11Device&);
        ~PredefinedResources();

        wrl::ComPtr<ID3D11ShaderResourceView> GetWhite() const { return white_; }

        MvpTransformVS GetBasicVS() const { return basicVS_; }
        SimpleLightingPS GetBasicPS() const { return basicPS_; }

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
        MvpTransformVS basicVS_;
        SimpleLightingPS basicPS_;

        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_, drawnOnly_, noDoubleBlending_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_, transparent_;
        wrl::ComPtr<ID3D11RasterizerState> cullClockWise_, wireFrameOnly_;

        wrl::ComPtr<ID3D11InputLayout> simpleLayout_;
    };

    struct BasicDrawContext;

    void SetupBasicLighting(const BasicDrawContext& drawContext,
        SimpleLightingPS& ps,
        const Smoothness& smoothness,
        ID3D11ShaderResourceView* tex = nullptr,
        ID3D11SamplerState* sampler = nullptr);

    void UpdateMvpCb(const BasicDrawContext& drawContext, const DirectX::XMMATRIX& world, MvpTransformVS& vs);

    struct BasicPipeline
    {
        enum : std::size_t
        {
            kPos, kNormal, kTangent, kTexCoord
        };

        wrl::ComPtr<ID3D11InputLayout> Layout;
        MvpTransformVS VS;
        SimpleLightingPS PS;
        std::array<VertexBuffer, 4> Vbs;
        ConstIndexBuffer Ib;
        wrl::ComPtr<ID3D11ShaderResourceView> Texture;
        wrl::ComPtr<ID3D11SamplerState> Sampler;

        VertexBuffer& PosVb() { return Vbs[kPos]; }
        VertexBuffer& NormalVb() { return Vbs[kNormal]; }
        VertexBuffer& TangentVb() { return Vbs[kTangent]; }
        VertexBuffer& TexCoordVb() { return Vbs[kTexCoord]; }
    };

    void Bind(ID3D11DeviceContext& context3D, BasicPipeline& pipeline);
    
    using BasicObject = Object<BasicPipeline>;

    struct BasicDrawContext
    {
        ID3D11DeviceContext& Context;
        const Camera& Camera;
        gsl::span<const dx::Light> Lights;
    };

    void UpdateAndDraw(const BasicDrawContext& drawContext, const BasicObject& object);

    template<typename PipelineT>
    void DrawAllIndexed(ID3D11DeviceContext& context3D, const PipelineT& pipeline)
    {
        context3D.DrawIndexed(pipeline.Ib.CountOfIndices(), 0, 0);
    }
}