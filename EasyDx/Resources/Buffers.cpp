#include "../pch.hpp"
#include "Buffers.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>

namespace dx
{
    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(
            ::ID3D11Device& device,
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

    void UpdateGpuBuffer(ID3D11DeviceContext& context, ID3D11Buffer& buffer, gsl::span<const std::byte> bytes)
    {
        context.UpdateSubresource(&buffer, 0, nullptr, bytes.data(), 0, 0);
    }

    void Bind(ID3D11DeviceContext& context3D, const ImmutableIndexBuffer& ib)
    {
        context3D.IASetIndexBuffer(ib.D3DBuffer(), static_cast<DXGI_FORMAT>(ImmutableIndexBuffer::kFormat), 0);
    }
}