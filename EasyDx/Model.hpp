#pragma once

#include "FileSystemAlias.hpp"
#include "DXDef.hpp"
#include "SimpleVertex.hpp"

namespace dx
{
    class Mesh;

    template<typename T>
    struct MeshData;

    void LoadFromModel(ID3D11Device& device, const fs::path& filePath, std::vector<Mesh>& meshes);

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
        std::uint16_t stackCount, MeshData<SimpleVertex>& meshData);

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount, 
        MeshData<SimpleVertex>& meshData);

    void MakeIcoSphere(std::uint32_t recursionLevel,
        MeshData<SimpleVertex>& meshData);
}