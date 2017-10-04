#pragma once

#include "Buffers.hpp"
#include "Shaders.hpp"
#include "SimpleVertex.hpp"

namespace dx
{
    template<typename T>
    struct CpuMeshView;

    template<typename T>
    struct CpuMesh 
    {
        std::vector<T> Vertices;
        std::vector<std::uint16_t> Indices;

        CpuMesh() = default;

        CpuMesh(std::vector<T> vertices, std::vector<std::uint16_t> indices)
            : Vertices{std::move(vertices)},
            Indices{std::move(indices)}
        {}

        CpuMeshView<T> Get() const noexcept
        {
            return { gsl::make_span(Vertices), gsl::make_span(Indices) };
        }
    };

    template<typename T>
    struct CpuMeshView
    {
        gsl::span<const T> Vertices;
        gsl::span<const std::uint16_t> Indices;

        CpuMeshView(gsl::span<const T> vertices, gsl::span<const std::uint16_t> indices)
            : Vertices{vertices},
            Indices{indices}
        {}

        template<std::size_t VN, std::size_t IN_>
        CpuMeshView(const T (&vertices)[VN], const std::uint16_t (&indices)[IN_])
            : Vertices{gsl::make_span(vertices)},
            Indices{gsl::make_span(indices)}
        {}
    };

    using SimpleCpuMesh = CpuMesh<SimpleVertex>;
    using SimpleCpuMeshView = CpuMeshView<SimpleVertex>;

    extern template struct CpuMesh<SimpleVertex>;
    extern template struct CpuMeshView<SimpleVertex>;

    struct GpuMeshView
    {
        VertexBufferView VertexBuffer;
        ID3D11Buffer* IndexBuffer;
        std::uint16_t IndexCount;
    };

    struct GpuMesh
    {
        GpuMesh() = default;

        template<typename VertexT>
        GpuMesh(ID3D11Device& device3D,
            CpuMeshView<VertexT> meshData,
            ResourceUsage usage = ResourceUsage::Immutable)
            : vertexBuffer_{ MakeVertexBuffer(device3D, meshData.Vertices, usage) },
            indexBuffer_{MakeIndexBuffer(device3D, meshData.Indices, usage)},
            indexCount_{static_cast<std::uint16_t>(meshData.Indices.size())}
        {}

        GpuMeshView Get() const noexcept
        {
            return {
                vertexBuffer_.Get(),
                indexBuffer_.Get(),
                indexCount_,
            };
        }

    private:
        SharedVertexBuffer vertexBuffer_;
        wrl::ComPtr<ID3D11Buffer> indexBuffer_;
        std::uint16_t indexCount_;
    };

    void SetupGpuMesh(ID3D11DeviceContext& context, GpuMeshView mesh);
}