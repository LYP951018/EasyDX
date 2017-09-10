#pragma once

#include "Component.hpp"

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

    struct VertexBufferView
    {
        ID3D11Buffer* Buffer;
        std::uint32_t VertexStride;
        std::uint32_t VertexCount;
    };

    struct SharedVertexBuffer
    {
        SharedVertexBuffer(wrl::ComPtr<ID3D11Buffer> buffer,
            std::uint32_t vertexStride, std::uint16_t vertexCount) noexcept;

        wrl::ComPtr<ID3D11Buffer> Buffer;
        //TODO: 16£¿
        std::uint32_t VertexStride;
        std::uint16_t VertexCount;

        VertexBufferView Get() const noexcept;
    };

    template<typename VertexT>
    SharedVertexBuffer MakeDynamicVertexBuffer(ID3D11Device& device, std::uint16_t vertexCount)
    {
        return {
            Internal::RawMakeD3DBuffer(device, {}, sizeof(VertexT) * static_cast<std::size_t>(vertexCount),
                BindFlag::VertexBuffer, ResourceUsage::Dynamic),
                static_cast<std::uint32_t>(sizeof(VertexT)),
                vertexCount
        };
    }

    template<typename VertexT>
    SharedVertexBuffer MakeVertexBuffer(
        ID3D11Device& device,
        gsl::span<VertexT> vertices,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return {Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.length_bytes(), BindFlag::VertexBuffer, usage), 
            static_cast<std::uint32_t>(sizeof(VertexT)),
            static_cast<std::uint16_t>(vertices.size())};
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

    void UpdateConstantBuffer(ID3D11DeviceContext& context, ID3D11Buffer& buffer, gsl::span<const std::byte> bytes);

    void SetupVSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot = 0);
    void SetupPSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot = 0);
    void SetupVertexBuffer(ID3D11DeviceContext& deviceContext, VertexBufferView vb);
}