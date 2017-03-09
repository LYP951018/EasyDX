#pragma once

#include "Common.hpp"
#include "Buffers.hpp"
#include <gsl/span>

namespace dx
{
    class Mesh
    {
    public:
        Mesh(std::uint32_t vertexStride,
            std::uint32_t indicesNum,
            wrl::ComPtr<ID3D11Buffer> vertexBuffer,
            wrl::ComPtr<ID3D11Buffer> indexBuffer);

        template<typename VertexT>
        Mesh(ID3D11Device& device,
            gsl::span<VertexT> vertices,
            gsl::span<std::uint16_t> indices,
            BufferUsage usage = BufferUsage::Default)
            : Mesh{ static_cast<std::uint32_t>(sizeof(VertexT)),
            static_cast<std::uint32_t>(indices.size()),
            MakeVertexBuffer(device, vertices, usage),
            MakeIndexBuffer(device, indices, usage) }
        {}

        void Render(ID3D11DeviceContext& context);

    private:
        std::uint32_t vertexStride_;
        std::uint32_t indicesNum_;
        wrl::ComPtr<ID3D11Buffer> vertexBuffer_;
        wrl::ComPtr<ID3D11Buffer> indexBuffer_;
    };
}