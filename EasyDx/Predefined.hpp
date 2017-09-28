#pragma once

#include "Shaders.hpp"
#include "Texture.hpp"
#include "CBStructs.hpp"
#include "Behavior.hpp"

namespace dx
{
    struct Renderable;
    class Camera;

    namespace cb::data
    {
        struct alignas(16) GlobalLightingInfo
        {
            DirectX::XMFLOAT3 EyePos;
            float Padding;
            ::dx::cb::data::Light Lights[10];
        };

        struct alignas(16) PerObjectLightingInfo
        {
            ::dx::cb::data::Material Smoothness;
        };

        struct alignas(16) BasicCb
        {
            DirectX::XMMATRIX WorldViewProj;
            DirectX::XMMATRIX World;
            DirectX::XMMATRIX WorldInvTranspose;
        };
    }

    namespace cb
    {
        using Basic = Cb<data::BasicCb>;
        using CbGlobalLightingInfo = dx::Cb<cb::data::GlobalLightingInfo>;
        using CbPerObjectLightingInfo = dx::Cb<cb::data::PerObjectLightingInfo>;

    }

    struct BasicCbUpdator : Behavior
    {
        const Camera& Camera_;

        BasicCbUpdator(const Camera& camera)
            : Camera_{ camera }, Behavior{ Behavior::kCbUpdate }
        {}

        void Update(GameObject&, const UpdateArgs&) override;
    };


    class Predefined
    {
    public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        Predefined(ID3D11Device&);
        void SetupCamera(const Camera&);

        Rc<Texture> GetWhite() const { return white_; }
        Rc<Renderable> GetRenderable(gsl::span<SimpleVertex> vertices, gsl::span<std::uint16_t> indices) const;
        Rc<Cb<cb::data::BasicCb>> GetBasicVSCpuCb() const { return vsCb_.first; }
        Rc<Cb<cb::data::PerObjectLightingInfo>> GetBasicLightingCpuCb() const noexcept { return psPerObjectLightingCb_.first; }
        Rc<BasicCbUpdator> GetBasicCbUpdator() const;
        VertexShader GetBasicVS() const { return basicVS_; }
        PixelShader GetBasicPS() const { return basicPS_; }
        wrl::ComPtr<ID3D11DepthStencilState> GetStencilAlways() const { return stencilAlways_; }
        wrl::ComPtr<ID3D11BlendState> GetNoWriteToRT() const { return noWriteToRt_; }

    private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeBasicVS(ID3D11Device&);
        void MakeBasicPS(ID3D11Device&);
        void MakeStencilStates(ID3D11Device&);
        void MakeBlendingStates(ID3D11Device&);

        static VertexShader MakeVS(ID3D11Device& device, const char* fileName);
        static PixelShader MakePS(ID3D11Device& device, const char* fileName);

        Rc<Texture> white_;
        VertexShader basicVS_;
        PixelShader basicPS_;
        CbPair<cb::data::BasicCb> vsCb_;
        CbPair<cb::data::PerObjectLightingInfo> psPerObjectLightingCb_;
        Rc<BasicCbUpdator> basicCbUpdator_;
        wrl::ComPtr<ID3D11DepthStencilState> stencilAlways_;
        wrl::ComPtr<ID3D11BlendState> noWriteToRt_;
    };

    
    
}