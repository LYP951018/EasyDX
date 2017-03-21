#pragma once

#include "Common.hpp"
#include "Buffers.hpp"
#include "Shaders.hpp"
#include <gsl/span>

namespace dx
{
    struct Material;

    class Mesh
    {
    public:
        Mesh(std::uint32_t vertexStride,
            std::uint32_t indicesNum,
            wrl::ComPtr<ID3D11Buffer> vertexBuffer,
            wrl::ComPtr<ID3D11Buffer> indexBuffer,
            std::shared_ptr<Material> material);

        template<typename VertexT>
        Mesh(ID3D11Device& device,
            gsl::span<VertexT> vertices,
            gsl::span<std::uint16_t> indices,
            std::shared_ptr<Material> material,
            ResourceUsage usage = ResourceUsage::Default)
            : Mesh{ static_cast<std::uint32_t>(sizeof(VertexT)),
            static_cast<std::uint32_t>(indices.size()),
            MakeVertexBuffer(device, vertices, usage),
            MakeIndexBuffer(device, indices, usage),
            std::move(material)}
        {}

        void Render(ID3D11DeviceContext& context);
        void AttachShaders(VertexShader vertexShader, wrl::ComPtr<ID3D11PixelShader> pixelShader);

    private:
        std::uint32_t vertexStride_;
        std::uint32_t indicesNum_;
        wrl::ComPtr<ID3D11Buffer> vertexBuffer_;
        wrl::ComPtr<ID3D11Buffer> indexBuffer_;
        std::shared_ptr<Material> material_;
        VertexShader vertexShader_;
        wrl::ComPtr<ID3D11PixelShader> pixelShader_;
    };
}