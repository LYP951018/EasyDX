#pragma once

#include "FileSystemAlias.hpp"
#include "DXDef.hpp"
#include "SimpleVertex.hpp"

namespace dx
{
    class GpuMeshData;

    template<typename T>
    struct CpuMesh;

    class GameObject;

    void LoadFromModel(ID3D11Device& device, const fs::path& filePath,
        std::vector<std::shared_ptr<GameObject>>& objects);

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
        std::uint16_t stackCount, CpuMesh<SimpleVertex>& meshData);

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount, 
        CpuMesh<SimpleVertex>& meshData);

    void MakeIcoSphere(std::uint32_t recursionLevel,
        CpuMesh<SimpleVertex>& meshData);
}