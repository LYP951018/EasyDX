#pragma once

#include "Common.hpp"
#include "Buffers.hpp"
#include "Shaders.hpp"
#include "SimpleVertex.hpp"
#include <gsl/span>
#include <vector>

namespace dx
{
    struct Material;

    template<typename T>
    struct MeshData
    {
        std::vector<T> Vertices;
        std::vector<std::uint16_t> Indices;
    };

    template<typename T>
    struct MeshDataView
    {
        gsl::span<const T> Vertices;
        gsl::span<const std::uint16_t> Indices;

        static MeshDataView FromMeshData(const MeshData<T>& meshData) noexcept
        {
            return MeshDataView{ gsl::make_span(meshData.Vertices), gsl::make_span(meshData.Indices) };
        }
    };

    using SimpleMeshData = MeshData<SimpleVertex>;
    using SimpleMeshDataView = MeshDataView<SimpleVertex>;

    extern template struct MeshData<SimpleVertex>;
    extern template struct MeshDataView<SimpleVertex>;

    class Mesh
    {
    public:
        Mesh(std::uint32_t indicesNum,
            SharedVertexBuffer vertexBuffer,
            wrl::ComPtr<ID3D11Buffer> indexBuffer,
            std::shared_ptr<Material> material,
            VertexShader vs,
            PixelShader ps);

        template<typename VertexT>
        Mesh(ID3D11Device& device,
            MeshDataView<VertexT> meshData,
            std::shared_ptr<Material> material,
            VertexShader vs,
            PixelShader ps,
            ResourceUsage usage = ResourceUsage::Default)
            : Mesh{static_cast<std::uint32_t>(meshData.Indices.size()),
            MakeVertexBuffer(device, meshData.Vertices, usage),
            MakeIndexBuffer(device, meshData.Indices, usage),
            std::move(material),
            std::move(vs), std::move(ps)}
        {}

        void DrawIndexed(ID3D11DeviceContext& context) const;
        void DrawIndexedInstanced(ID3D11DeviceContext& context, VertexBuffer, std::uint32_t instanceCount) const;
        VertexBuffer GetVertexBuffer() const noexcept;
        ID3D11Buffer& GetIndexBuffer() const noexcept;
        std::uint32_t GetIndicesCount() const noexcept;
        const Material* GetMaterial() const noexcept;
        VertexShaderView GetVertexShader() const noexcept;
        PixelShaderView GetPixelShader() const noexcept;

    private:
        std::uint32_t indicesNum_;
        SharedVertexBuffer vertexBuffer_;
        wrl::ComPtr<ID3D11Buffer> indexBuffer_;
        std::shared_ptr<Material> material_;
        VertexShader vertexShader_;
        PixelShader pixelShader_;

        void SetupShaders(ID3D11DeviceContext& deviceContext) const;
        void SetupIndexAndTopo(ID3D11DeviceContext& deviceContext) const;
    };
}