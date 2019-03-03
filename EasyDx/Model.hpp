#pragma once

#include "DXDef.hpp"
#include "Vertex.hpp"
#include "Resources/Buffers.hpp"
#include "EasyDx.Common/FlagEnums.hpp"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <d3d11.h>

struct aiMesh;
struct aiMaterial;
struct aiScene;

namespace dx
{
    using PositionType = DirectX::XMFLOAT3A;
    using VectorType = DirectX::XMFLOAT3A;
    using TexCoordType = DirectX::XMFLOAT2A;
    using ColorType = DirectX::XMFLOAT4;

    using PositionStream = std::vector<PositionType>;
    using VectorStream = std::vector<VectorType>;
    using TexCoordStream = std::vector<TexCoordType>;
    using ColorStream = std::vector<ColorType>;

    PositionType MakePosition(float x, float y, float z);
    VectorType MakeDir(float x, float y, float z, float w = 0.0f);
    TexCoordType MakeTexCoord(float x, float y);
    ColorType MakeColor(float r, float g, float b, float a);

    inline VectorType StoreVec(DirectX::XMVECTOR vec) { return Store<VectorType>(vec); }

    enum class LoadFlags : std::uint32_t
    {
        kNone = 0,
        kPositions = 1,
        kNormals = 1 << 1,
        kTangents = 1 << 2,
        kColors = 1 << 3,
        kTexCoords = 1 << 4,
        kPosNormal = kPositions | kNormals,
        kPosNormalTexCoord = kPositions | kNormals | kTexCoords,
        kPosNormalTangent = kPositions | kNormals | kTangents,
        kPosNormalTangentTexCoord = kPosNormalTangent | kTexCoords,
        kAll = kPositions | kNormals | kTangents | kColors | kTexCoords
    };

    struct LoadedMesh
    {
        PositionStream Positions;
        VectorStream Normals;
        VectorStream Tangents;
        // VectorStream Bitangents;
        ColorStream Colors;
        TexCoordStream TexCoords;
        std::vector<ShortIndex> Indices;

        void Reserve(std::size_t size);
        void Clear();
    };

    std::true_type UseEnumFlag(LoadFlags);

    struct Smoothness;
    class Mesh;

    void IndicesFromMesh(const aiMesh& mesh, std::vector<ShortIndex>& indices);

    std::optional<Smoothness> SmoothnessFromMaterial(const aiMaterial& material);

    // TODO: VSSemantics expectedSemantics
    std::shared_ptr<Mesh> ConvertToImmutableMesh(ID3D11Device& device3D, const aiMesh& aiMesh_);
    D3D11_PRIMITIVE_TOPOLOGY AsD3DPrimitiveTopology(aiPrimitiveType primitiveType);

    D3D11_TEXTURE_ADDRESS_MODE FromAssimpTexMapModeToDX(aiTextureMapMode mode);

    void FillAddressModes(const aiMaterial& material, std::uint32_t n,
                          D3D11_SAMPLER_DESC& samplerDesc,
                          aiTextureType texUsage = aiTextureType::aiTextureType_DIFFUSE);

    void GetTexPath(const aiMaterial& material, std::uint32_t n, fs::path& path,
                    aiTextureType texUsage = aiTextureType::aiTextureType_DIFFUSE);

    auto GetMaterialsInScene(const aiScene& scene) -> gsl::span<const Ptr<const aiMaterial>>;
    auto GetMeshesInScene(const aiScene& scene) -> gsl::span<const Ptr<const aiMesh>>;

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
                      std::uint16_t stackCount, LoadedMesh& meshData);

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount,
                      LoadedMesh& meshData);

    /*void MakeIcoSphere(std::uint32_t recursionLevel,
        CpuMesh<SimpleVertex>& meshData);*/
} // namespace dx