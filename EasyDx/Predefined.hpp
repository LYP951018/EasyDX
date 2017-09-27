#pragma once

#include "Shaders.hpp"
#include "Texture.hpp"
#include "CBStructs.hpp"

namespace dx
{
    struct Renderable;

    class Predefined
    {
    public:
        static constexpr std::uint32_t kDefaultTexWidth = 100;
        static constexpr std::uint32_t kDefaultTexHeight = 100;

        Predefined(ID3D11Device&);

        Rc<Texture> GetWhite() const { return white_; }
        Rc<Renderable> GetRenderable(gsl::span<SimpleVertex> vertices, gsl::span<std::uint16_t> indices) const;

    private:
        void MakeWhiteTex(ID3D11Device&);
        void MakeBasicVS(ID3D11Device&);
        void MakeBasicPS(ID3D11Device&);

        static VertexShader MakeVS(ID3D11Device& device, const char* fileName);
        static PixelShader MakePS(ID3D11Device& device, const char* fileName);

        Rc<Texture> white_;
        VertexShader basicVS_;
        PixelShader basicPS_;
        CbPair<cb::data::BasicCb> vsCb_, psPerObjectLightingCb_;
    };

    namespace data
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
    }

    using CbGlobalLightingInfo = dx::Cb<data::GlobalLightingInfo>;
    using CbPerObjectLightingInfo = dx::Cb<data::PerObjectLightingInfo>;
}