#pragma once

#include "FileSystemAlias.hpp"
#include "DXDef.hpp"
#include "SimpleVertex.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include <tuple>
#include <experimental/generator>

namespace dx
{
    class GpuMeshData;

    template<typename T>
    struct CpuMesh;

    auto LoadFromModel(ID3D11Device& device, const fs::path& filePath) ->
        std::experimental::generator<
        std::tuple<SimpleCpuMesh, TextureSampler, Smoothness>>;

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
        std::uint16_t stackCount, CpuMesh<SimpleVertex>& meshData);

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount, 
        CpuMesh<SimpleVertex>& meshData);

    void MakeIcoSphere(std::uint32_t recursionLevel,
        CpuMesh<SimpleVertex>& meshData);
}