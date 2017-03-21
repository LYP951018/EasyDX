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

    template<typename VertexT>
    wrl::ComPtr<ID3D11Buffer> MakeVertexBuffer(
        ID3D11Device& device,
        gsl::span<VertexT> vertices,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.length_bytes(), BindFlag::VertexBuffer, usage);
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
}