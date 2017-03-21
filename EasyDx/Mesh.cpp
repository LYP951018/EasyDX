#include "Mesh.hpp"
#include "Material.hpp"
#include <gsl/span>
#include <d3d11.h>

namespace dx
{
    Mesh::Mesh(std::uint32_t vertexStride, 
        std::uint32_t indicesNum, 
        wrl::ComPtr<ID3D11Buffer> vertexBuffer, 
        wrl::ComPtr<ID3D11Buffer> indexBuffer,
        std::shared_ptr<Material> material)
        :vertexStride_{vertexStride},
        indicesNum_{ indicesNum },
        vertexBuffer_{ std::move(vertexBuffer) },
        indexBuffer_{ std::move(indexBuffer) },
        material_{std::move(material)}
    {
    }

    void Mesh::AttachShaders(VertexShader vertexShader, wrl::ComPtr<ID3D11PixelShader> pixelShader)
    {
        vertexShader_ = std::move(vertexShader);
        pixelShader_ = std::move(pixelShader);
    }

    void Mesh::Render(ID3D11DeviceContext& deviceContext)
    {
        ID3D11Buffer* const vertexBuffers[] = { vertexBuffer_.Get() };
        const UINT vertexBufferStrides[] = { vertexStride_ };
        const UINT vertexBufferOffsets[] = { 0 };
        deviceContext.IASetVertexBuffers(0, std::size(vertexBuffers), vertexBuffers, vertexBufferStrides, vertexBufferOffsets);
        deviceContext.IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
        deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        BindShaders(deviceContext, vertexShader_, *pixelShader_.Get());

        if (material_)
        {
            //const auto resources = ComPtrsCast(gsl::make_span(material_->Textures));
            const auto& texture = material_->MainTexture;
            ID3D11ShaderResourceView* resources[] = { texture.first.Get() };
            deviceContext.PSSetShaderResources(0, 1, resources);
            ID3D11SamplerState* samplers[] = { texture.second.Get() };
            deviceContext.PSSetSamplers(0, 1, samplers);
        }
        deviceContext.DrawIndexed(indicesNum_, 0, 0);
    }
}
