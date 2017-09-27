#pragma once

#include "Light.hpp"
#include "Buffers.hpp"
#include <DirectXMath.h>
#include <cstdint>

namespace dx
{
    struct Smoothness;

    using GpuCb = wrl::ComPtr<ID3D11Buffer>;

    struct IConstantBuffer
    {
        IConstantBuffer(wrl::ComPtr<ID3D11Buffer> gpuCb)
            : gpuCb_{std::move(gpuCb)}
        {}

        virtual gsl::span<const std::byte> GetBuffer() = 0;

        void Flush(ID3D11DeviceContext& context) noexcept
        {
            UpdateConstantBuffer(context, Ref(gpuCb_), GetBuffer());
        }

        virtual ~IConstantBuffer();

    private:
        wrl::ComPtr<ID3D11Buffer> gpuCb_;
    };

    template<typename CbDataT>
    struct Cb : IConstantBuffer
    {
    public:
        Cb(wrl::ComPtr<ID3D11Buffer> gpuCb)
            : data_{ aligned_unique<CbDataT>() },
            IConstantBuffer{std::move(gpuCb)}
        {}

        gsl::span<const std::byte> GetBuffer() override
        {
            return AsBytes(Data());
        }

        CbDataT& Data() noexcept
        {
            return *data_;
        }

        const CbDataT& Data() const noexcept
        {
            return *data_;
        }

    private:
        aligned_unique_ptr<CbDataT> data_;
        
    };

    template<typename CbDataT>
    using CbPair = std::pair<Rc<Cb<CbDataT>>, wrl::ComPtr<ID3D11Buffer>>;

    template<typename CbDataT>
    auto MakeCb(ID3D11Device& device) -> CbPair<CbDataT>
    {
        auto gpuCb = MakeConstantBuffer<CbDataT>(device);
        return {
            MakeShared<Cb<CbDataT>>(gpuCb),
            gpuCb
        };
    }

    namespace cb::data
    {
        struct alignas(16) Light
        {
            DirectX::XMFLOAT4 Position;
            DirectX::XMFLOAT4 Direction;
            DirectX::XMFLOAT4 Color;

            float SpotAngle;
            float ConstantAttenuation;
            float LinearAttenuation;
            float QuadraticAttenuation;

            LightType Type;
            bool Enabled;
            float Range;
            std::uint32_t Padding;

            void FromPoint(const dx::PointLight& point) noexcept;
            void FromDirectional(const dx::DirectionalLight& directional) noexcept;
            void FromSpot(const dx::SpotLight& spot) noexcept;
            void FromLight(const dx::Light& light) noexcept;
        };

        struct alignas(16) Material
        {
            DirectX::XMFLOAT4 Ambient;
            DirectX::XMFLOAT4 Diffuse;
            DirectX::XMFLOAT4 Specular;
            DirectX::XMFLOAT4 Emissive;

            float SpecularPower;
            DirectX::XMFLOAT3 Padding;

            void FromSmoothness(const Smoothness& smoothness) noexcept;
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
        using Light = Cb<data::Light>;
        using Material = Cb<data::Material>;
        using Basic = Cb<data::BasicCb>;
    }
}