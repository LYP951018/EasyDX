#pragma once

#include "DXDef.hpp"
#include "Vertex.hpp"
#include "Resources/Buffers.hpp"

namespace dx
{
    struct ModelResultUnit
    {
        PositionStream Positions;
        VectorStream Normals;
        VectorStream Tangents;
        VectorStream Bitangents;
        ColorStream Colors;
        TexCoordStream TexCoords;
        std::vector<ShortIndex> Indices;

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