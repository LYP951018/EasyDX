#include "pch.hpp"
#include "Buffers.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>

namespace dx
{
    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(
            ID3D11Device& device,
            const void* buffer,
            std::size_t bufferSize,
            BindFlag bindFlags,
            ResourceUsage usage)
        {
            Ensures(buffer != nullptr || usage != ResourceUsage::Immutable);
            wrl::ComPtr<ID3D11Buffer> d3dBuffer;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = static_cast<D3D11_USAGE>(usage);
            bufferDesc.ByteWidth = static_cast<UINT>(bufferSize);
            bufferDesc.BindFlags = static_cast<UINT>(bindFlags);
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = buffer;
            dx::TryHR(device.CreateBuffer(&bufferDesc, buffer == nullptr ? nullptr : &initData, d3dBuffer.GetAddressOf()));
            return d3dBuffer;
        }
    }

    void UpdateConstantBuffer(ID3D11DeviceContext& context, ID3D11Buffer& buffer, gsl::span<const std::byte> bytes)
    {
        context.UpdateSubresource(&buffer, 0, nullptr, bytes.data(), 0, 0);
    }

    void SetupVSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot)
    {
        deviceContext.VSSetConstantBuffers(static_cast<UINT>(startSlot), static_cast<UINT>(cbuffers.size()), cbuffers.data());
    }

    void SetupPSConstantBuffer(ID3D11DeviceContext& deviceContext, gsl::span<const Ptr<ID3D11Buffer>> cbuffers, std::uint32_t startSlot)
    {
        deviceContext.PSSetConstantBuffers(static_cast<UINT>(startSlot), static_cast<UINT>(cbuffers.size()), cbuffers.data());
    }

    void SetupVertexBuffer(ID3D11DeviceContext& deviceContext, VertexBufferView vb)
    {
        Ptr<ID3D11Buffer> buffers[] = { vb.Buffer };
        UINT offsets[] = { 0 };
        UINT strides[] = { vb.VertexStride };
        deviceContext.IASetVertexBuffers(0, 1, buffers, strides, offsets);
    }

    VertexBufferView SharedVertexBuffer::Get() const noexcept
    {
        return { Buffer.Get(), VertexStride, VertexCount };
    }

    SharedVertexBuffer::SharedVertexBuffer(wrl::ComPtr<ID3D11Buffer> buffer,
        std::uint32_t vertexStride, std::uint16_t vertexCount) noexcept
        : Buffer{ std::move(buffer) },
        VertexStride{ vertexStride },
        VertexCount{ vertexCount }
    {}
}