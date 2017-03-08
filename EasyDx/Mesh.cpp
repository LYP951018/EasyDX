#include "Mesh.hpp"
#include <d3d11.h>

namespace dx
{
    Mesh::Mesh(std::uint32_t vertexStride, 
        std::uint32_t indicesNum, 
        wrl::ComPtr<ID3D11Buffer> vertexBuffer, 
        wrl::ComPtr<ID3D11Buffer> indexBuffer)
        :vertexStride_{vertexStride},
        indicesNum_{ indicesNum },
        vertexBuffer_{ std::move(vertexBuffer) },
        indexBuffer_{ std::move(indexBuffer) }
    {
    }

    void Mesh::Render(ID3D11DeviceContext & deviceContext)
    {
        ID3D11Buffer* const vertexBuffers[] = { vertexBuffer_.Get() };
        const UINT vertexBufferStrides[] = { vertexStride_ };
        const UINT vertexBufferOffsets[] = { 0 };
        deviceContext.IASetVertexBuffers(0, std::size(vertexBuffers), vertexBuffers, vertexBufferStrides, vertexBufferOffsets);
        deviceContext.IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
        deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        deviceContext.DrawIndexed(indicesNum_, 0, 0);
    }
}
