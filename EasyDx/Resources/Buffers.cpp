#include "../pch.hpp"
#include "Buffers.hpp"
#include <d3d11.h>
#include <gsl/gsl_assert>

namespace dx
{
    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(::ID3D11Device& device,
                                                   const void* buffer,
                                                   std::uint32_t bufferSize,
                                                   BindFlag bindFlags,
                                                   ResourceUsage usage)
        {
            Ensures(buffer != nullptr || usage != ResourceUsage::Immutable);
            wrl::ComPtr<ID3D11Buffer> d3dBuffer;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = static_cast<D3D11_USAGE>(usage);
            bufferDesc.ByteWidth = static_cast<UINT>(bufferSize);
            bufferDesc.BindFlags = static_cast<UINT>(bindFlags);
            if (usage == ResourceUsage::Dynamic)
            {
                bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            }
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = buffer;
            dx::TryHR(device.CreateBuffer(
                &bufferDesc, buffer == nullptr ? nullptr : &initData,
                d3dBuffer.GetAddressOf()));
            return d3dBuffer;
        }
    } // namespace Internal

    D3D11_BUFFER_DESC GetDesc(ID3D11Buffer& buffer)
    {
        D3D11_BUFFER_DESC desc{};
        buffer.GetDesc(&desc);
        return desc;
    }

    MappedGpuResource Map(ID3D11DeviceContext& context, ID3D11Buffer& gpuBuffer,
                          ResourceMapType mapType)
    {
        D3D11_MAPPED_SUBRESOURCE data{};
        TryHR(context.Map(&gpuBuffer, 0, static_cast<D3D11_MAP>(mapType), 0,
                          &data));
        D3D11_BUFFER_DESC desc{};
        gpuBuffer.GetDesc(&desc);
        return MappedGpuResource{context, &gpuBuffer, data.pData,
                                 desc.ByteWidth};
    }

    void GpuBufferView::Update(ID3D11DeviceContext& context,
                               gsl::span<const std::byte> bytes)
    {
        Ensures(Count == bytes.size());
        D3D11_BOX box{};
        box.left = Offset;
        box.right = Offset + Count;
        box.top = 0;
        box.bottom = 1;
        box.front = 0;
        box.back = 1;
        context.UpdateSubresource(Buffer, 0, &box, bytes.data(), 0, 0);
    }

    void SetupIndexBuffer(ID3D11DeviceContext& context3D,
                          ID3D11Buffer& indexBuffer, std::uint32_t offset)
    {
        const auto desc = GetDesc(indexBuffer);
        // Ensures(desc.StructureByteStride == sizeof(ShortIndex));
        context3D.IASetIndexBuffer(
            &indexBuffer, static_cast<DXGI_FORMAT>(DxgiFormat::R16UInt),
            offset);
    }

    MappedGpuResource::~MappedGpuResource() { m_context->Unmap(m_resource, 0); }
} // namespace dx