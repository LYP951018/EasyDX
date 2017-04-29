#pragma once

#include "Common.hpp"
#include <gsl/span>
#include <cstdint>

namespace dx
{
    namespace Internal
    {
        extern wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(
            ID3D11Device& device,
            const void* buffer,
            std::size_t bufferSize,
            BindFlag bindFlags,
            ResourceUsage usage);
    }

    struct VertexBuffer
    {
        ID3D11Buffer* Buffer;
        std::uint32_t VertexStride;
    };

    struct SharedVertexBuffer
    {
        wrl::ComPtr<ID3D11Buffer> Buffer;
        std::uint32_t VertexStride;

        VertexBuffer Get() const noexcept;
    };

    template<typename VertexT>
    SharedVertexBuffer MakeVertexBuffer(
        ID3D11Device& device,
        gsl::span<VertexT> vertices,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return {Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.length_bytes(), BindFlag::VertexBuffer, usage), static_cast<std::uint32_t>(sizeof(VertexT))};
    }

    template<typename IndexT>
    wrl::ComPtr<ID3D11Buffer> MakeIndexBuffer(
        ID3D11Device& device,
        gsl::span<IndexT> indices,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return Internal::RawMakeD3DBuffer(device, indices.data(), indices.length_bytes(), BindFlag::IndexBuffer, usage);
    }

    template<typename T>
    wrl::ComPtr<ID3D11Buffer> MakeConstantBuffer(
        ID3D11Device& device,
        T* cb = nullptr,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return Internal::RawMakeD3DBuffer(device, cb, sizeof(*cb), BindFlag::ConstantBuffer, usage);
    }

    void SetupVSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot = 0);
    void SetupPSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot = 0);
}