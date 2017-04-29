#include "Mesh.hpp"
#include "Material.hpp"
#include <gsl/span>
#include <d3d11.h>

namespace dx
{
    template struct MeshData<SimpleVertex>;
    template struct MeshDataView<SimpleVertex>;

    Mesh::Mesh(std::uint32_t indicesNum, 
        SharedVertexBuffer vertexBuffer, 
        wrl::ComPtr<ID3D11Buffer> indexBuffer,
        std::shared_ptr<Material> material,
        VertexShader vs,
        PixelShader ps)
        :indicesNum_{ indicesNum },
        vertexBuffer_{ std::move(vertexBuffer) },
        indexBuffer_{ std::move(indexBuffer) },
        material_{std::move(material)},
        vertexShader_{std::move(vs)},
        pixelShader_{std::move(ps)}
    {
    }

    VertexBuffer Mesh::GetVertexBuffer() const noexcept
    {
        return vertexBuffer_.Get();
    }

    ID3D11Buffer& Mesh::GetIndexBuffer() const noexcept
    {
        return Ref(indexBuffer_);
    }

    std::uint32_t Mesh::GetIndicesCount() const noexcept
    {
        return indicesNum_;
    }

    const Material* Mesh::GetMaterial() const noexcept
    {
        return material_.get();
    }

    VertexShaderView Mesh::GetVertexShader() const noexcept
    {
        return vertexShader_.Get();
    }

    PixelShaderView Mesh::GetPixelShader() const noexcept
    {
        return pixelShader_.Get();
    }

    void Mesh::SetupShaders(ID3D11DeviceContext& deviceContext) const
    {
        vertexShader_.Bind(deviceContext);
        pixelShader_.Bind(deviceContext);
    }

    void Mesh::SetupIndexAndTopo(ID3D11DeviceContext& deviceContext) const
    {
        deviceContext.IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
        deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Mesh::DrawIndexed(ID3D11DeviceContext& deviceContext) const
    {
        const Ptr<ID3D11Buffer> vertexBuffers[] = { vertexBuffer_.Buffer.Get() };
        const UINT vertexBufferStrides[] = { vertexBuffer_.VertexStride };
        const UINT vertexBufferOffsets[] = { 0 };
        deviceContext.IASetVertexBuffers(0, std::size(vertexBuffers), vertexBuffers, vertexBufferStrides, vertexBufferOffsets);
        SetupIndexAndTopo(deviceContext);
        SetupShaders(deviceContext);
        deviceContext.DrawIndexed(indicesNum_, 0, 0);
    }

    void Mesh::DrawIndexedInstanced(ID3D11DeviceContext& deviceContext, VertexBuffer instanceBuffer, std::uint32_t instanceCount) const
    {
        const auto vertexBuffer = GetVertexBuffer();
        const Ptr<ID3D11Buffer> vertexBuffers[] = { vertexBuffer.Buffer, instanceBuffer.Buffer };
        const UINT vertexBufferStrides[] = { vertexBuffer.VertexStride, instanceBuffer.VertexStride };
        const UINT vertexBufferOffsets[] = { 0, 0 };
        deviceContext.IASetVertexBuffers(0, std::size(vertexBuffers), vertexBuffers, vertexBufferStrides, vertexBufferOffsets);
        SetupIndexAndTopo(deviceContext);
        SetupShaders(deviceContext);
        deviceContext.DrawIndexedInstanced(indicesNum_, instanceCount, 0, 0, 0);
    }
}
