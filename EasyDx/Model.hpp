#pragma once

#include "DXDef.hpp"
#include "Vertex.hpp"
#include "Resources/Buffers.hpp"

namespace dx
{
    struct ModelResultUnit
    {
        Float4Stream Positions;
        Float4Stream Normals;
        Float4Stream Tangents;
        Float4Stream Bitangents;
        Float4Stream Colors;
        Float2Stream TexCoords;
        std::vector<Index> Indices;

        void Reserve(std::size_t size);
    };

    auto LoadFromModel(ID3D11Device& device, const fs::path& filePath)
        -> std::unordered_multimap<std::string, ModelResultUnit>;

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
        std::uint16_t stackCount, ModelResultUnit& meshData);

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount, 
        ModelResultUnit& meshData);

    /*void MakeIcoSphere(std::uint32_t recursionLevel,
        CpuMesh<SimpleVertex>& meshData);*/
}