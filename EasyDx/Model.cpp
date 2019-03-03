#include "pch.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Resources/Shaders.hpp"
#include "DxMathWrappers.hpp"
#include "Predefined.hpp"
#include "Misc.hpp"
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
#include <d3d11.h>

namespace dx
{
    void CheckAiReturn(aiReturn code)
    {
        ThrowIf<std::runtime_error>(code != aiReturn::aiReturn_SUCCESS, "Assimp error");
    }

    DirectX::XMFLOAT4 AiColorToFloat4(const aiColor4D& color) noexcept
    {
        return {color.r, color.g, color.b, color.a};
    }

    DirectX::XMFLOAT4 AiColorToFloat4(const aiColor3D& color) noexcept
    {
        return {color.r, color.g, color.b, 1.0f};
    }

    PositionType MakeAiPosition(const aiVector3D& vec3) noexcept
    {
        return MakePosition(vec3.x, vec3.y, vec3.z);
    }

    VectorType MakeAiDirection(const aiVector3D& vec3) noexcept
    {
        return MakeDir(vec3.x, vec3.y, vec3.z);
    }

    D3D11_TEXTURE_ADDRESS_MODE FromAssimpTexMapModeToDX(aiTextureMapMode mode)
    {
        switch (mode)
        {
        case aiTextureMapMode_Wrap:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case aiTextureMapMode_Clamp:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case aiTextureMapMode_Mirror:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case _aiTextureMapMode_Force32Bit:
        case aiTextureMapMode_Decal:
        default:
            throw std::runtime_error{"Unsupported map mode."};
            break;
        }
    }

    void FillAddressModes(const aiMaterial& material, std::uint32_t n,
                          D3D11_SAMPLER_DESC& samplerDesc, aiTextureType texUsage)
    {
        aiTextureMapMode uMapMode = aiTextureMapMode_Wrap, vMapMode = aiTextureMapMode_Wrap;
        material.Get(AI_MATKEY_MAPPINGMODE_U(texUsage, n), uMapMode);
        material.Get(AI_MATKEY_MAPPINGMODE_V(texUsage, n), vMapMode);
        samplerDesc.AddressU = FromAssimpTexMapModeToDX(uMapMode);
        samplerDesc.AddressV = FromAssimpTexMapModeToDX(vMapMode);
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    }

    void GetTexPath(const aiMaterial& material, std::uint32_t n, fs::path& path,
                    aiTextureType texUsage)
    {
        aiString texPath;
        CheckAiReturn(material.Get(AI_MATKEY_TEXTURE(texUsage, n), texPath));
        path = {texPath.C_Str()};
    }

    auto GetMaterialsInScene(const aiScene& scene) -> gsl::span<const Ptr<const aiMaterial>>
    {
        return gsl::make_span(scene.mMaterials, scene.mNumMaterials);
    }

    auto GetMeshesInScene(const aiScene& scene) -> gsl::span<const Ptr<const aiMesh>>
    {
        return gsl::make_span(scene.mMeshes, scene.mNumMeshes);
    }


	PositionType MakePosition(float x, float y, float z) { return PositionType{ x, y, z }; }

	VectorType MakeDir(float x, float y, float z, float w) { return VectorType{ x, y, z }; }

	TexCoordType MakeTexCoord(float x, float y) { return TexCoordType{ x, y }; }

	ColorType MakeColor(float r, float g, float b, float a) { return ColorType{ r, g, b, a }; }

	void IndicesFromMesh(const aiMesh& mesh, std::vector<ShortIndex>& indices)
    {
        const auto faces = gsl::make_span(mesh.mFaces, mesh.mNumFaces);
        indices.clear();
        indices.reserve(faces.size() * 3);
        for (const auto& face : faces)
        {
            const auto faceIndices = gsl::make_span(face.mIndices, face.mNumIndices);
           /* if (faceIndices.size() != 3)
                continue;
*/
            for (auto index : faceIndices)
            {
                indices.push_back(static_cast<std::uint16_t>(index));
            }
        }
    }

	std::optional<Smoothness> SmoothnessFromMaterial(const aiMaterial& material)
    {
        aiColor3D diffuse, specular, ambient, emissive;

#define TRY_ASSIMP(expr) if ((expr) != aiReturn::aiReturn_SUCCESS) return std::nullopt;
		TRY_ASSIMP(material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuse));
		TRY_ASSIMP(material.Get(AI_MATKEY_COLOR_SPECULAR, specular));
		TRY_ASSIMP(material.Get(AI_MATKEY_COLOR_AMBIENT, ambient));
		TRY_ASSIMP(material.Get(AI_MATKEY_COLOR_EMISSIVE, emissive));
        float specularPower;
		TRY_ASSIMP(material.Get(AI_MATKEY_SHININESS, specularPower)); 
		Smoothness smoothness;
        smoothness.Amibient = AiColorToFloat4(ambient);
        smoothness.Specular = AiColorToFloat4(specular);
        smoothness.Diffuse = AiColorToFloat4(diffuse);
		smoothness.Emissive = AiColorToFloat4(emissive);
        smoothness.SpecularPower = specularPower;
		return smoothness;
    }

    std::shared_ptr<Mesh> ConvertToImmutableMesh(ID3D11Device& device3D, const aiMesh& aiMesh_)
    {
        std::vector<ShortIndex> indices;
        IndicesFromMesh(aiMesh_, indices);
        //const VSSemantics semanticsMask = VSSemanticsFromMesh(aiMesh_);
        const std::uint32_t vertexCount = aiMesh_.mNumVertices;
		std::vector<gsl::span<const std::byte>> channels;
		std::vector<VSSemantics> semantices;
		VSSemantics allSemantics;
		std::vector<std::uint32_t> strides;
		std::vector<DxgiFormat> formats;
		std::vector<std::uint32_t> semanticsIndices;
		const auto pushChannel = [&](VSSemantics semantics, const auto p) {
			semantices.push_back(semantics);
			channels.push_back(gsl::as_bytes(gsl::make_span(p, aiMesh_.mNumVertices)));
			strides.push_back(sizeof(*p));
			formats.push_back(FormatFromSemantic(semantics));
			semanticsIndices.push_back(0);
		};

		pushChannel(VSSemantics::kPosition, aiMesh_.mVertices);
		if (aiMesh_.HasNormals())
		{
			pushChannel(VSSemantics::kNormal, aiMesh_.mNormals);
		}
		if (aiMesh_.HasTangentsAndBitangents())
		{
			pushChannel(VSSemantics::kTangent, aiMesh_.mTangents);
		}
		if (aiMesh_.HasVertexColors(0))
		{
			pushChannel(VSSemantics::kColor, aiMesh_.mColors[0]);
		}
		// TODO: multi-uv
		if (aiMesh_.HasTextureCoords(0))
		{
			pushChannel(VSSemantics::kTexCoord, aiMesh_.mTextureCoords[0]);
		}

		const D3D11_PRIMITIVE_TOPOLOGY topology = AsD3DPrimitiveTopology(static_cast<aiPrimitiveType>(aiMesh_.mPrimitiveTypes));
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementsDesces;
		FillInputElementsDesc(inputElementsDesces, semantices, formats, semanticsIndices);
		return std::make_shared<Mesh>(Mesh::CreateImmutable(device3D, channels.size(), channels.data(), strides.data(), semantices.data(), std::move(inputElementsDesces),
			gsl::span<const ShortIndex>{indices}, topology));
    }

    D3D11_PRIMITIVE_TOPOLOGY AsD3DPrimitiveTopology(aiPrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
        case aiPrimitiveType_POINT:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case aiPrimitiveType_LINE:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case aiPrimitiveType_TRIANGLE:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        /*case aiPrimitiveType_POLYGON:
                break;
        case _aiPrimitiveType_Force32Bit:
                break;*/
        default:
            assert(false);
            break;
        }
        assert(false);
    }

    void CheckStreamsSizeEqualityExceptColors(const LoadedMesh& unit)
    {
        auto& [positions, normals, tangents, colors, texCoords, indices] = unit;
        const auto sizes =
            std::array{positions.size(), normals.size(), tangents.size(), texCoords.size()};
        const auto first = sizes[0];
        Ensures(
            std::all_of(sizes.begin(), sizes.end(), [first](std::size_t s) { return s == first; }));
    }

    void MakeCylinder(float bottomRadius, float topRadius, float height, std::uint16_t sliceCount,
                      std::uint16_t stackCount, LoadedMesh& meshData)
    {
        using namespace DirectX;
        // auto& vertices = meshData.Positions;
        // auto& indices = meshData.Indices;
        // vertices.clear();
        const std::uint16_t sliceRingCount = sliceCount + 1;
        const std::uint16_t stackRingCount = stackCount + 1;
        const std::uint16_t verticesCount = (stackRingCount + 2) * sliceRingCount + 2;

        meshData.Reserve(verticesCount);
        const float heightPerStack = height / stackCount;
        const float angleStep = DirectX::XM_2PI / sliceCount;
        const auto radiusDelta = bottomRadius - topRadius;
        const float radiusStep = radiusDelta / stackCount;
        float currentHeight = 0.f;
        float radius = bottomRadius;
        auto& [positions, normals, tangents, colors, texCoords, indices] = meshData;
        // r(v) = bottomRadius - (bottomRadius - topRadius) * v
        // v = y / h
        // y(v) = h * v
        // x(t) = r(v) * cos(t)
        // y(t) = v
        // z(t) = r(v) * sin(t)
        // dx/dt = -r(v) * sin(t)
        // dy/dt = 0
        // dx/dv = (b - top) * cos(t)
        // dy/dv = h
        // dz/dv = (b - top) * sin(t)
        for (std::uint16_t i = 0; i < stackRingCount; ++i)
        {
            float angle = 0.f;
            for (std::uint16_t j = 0; j <= sliceCount; ++j)
            {
                const auto c = std::cos(angle);
                const auto s = std::sin(angle);
                positions.push_back(MakePosition(radius * c, currentHeight, radius * s));
                texCoords.push_back(
                    MakeTexCoord(1.f - currentHeight / height, angle / DirectX::XM_2PI));
                // normalized already.
                const auto tangentU = MakeDir(-s, 0, c);
                tangents.push_back(tangentU);
                const auto biTangent = MakeDir(radiusDelta * c, -height, radiusDelta * s);
                const auto tangentUVec = Load(tangentU);
                const auto biTangentVec = Load(biTangent);
                const auto biTangentUVec = XMVector3Normalize(biTangentVec);
                const auto normalUVec = XMVector3Cross(tangentUVec, biTangentVec);
                normals.push_back(StoreVec(normalUVec));
                angle += angleStep;
            }
            currentHeight += heightPerStack;
            radius -= radiusStep;
        }

        Expects(meshData.Positions.size() < static_cast<std::size_t>(UINT16_MAX));
        indices.clear();
        const std::uint16_t indicesCount = sliceCount * 3 * 2 + stackCount * sliceCount * 3 * 2;
        indices.reserve(indicesCount);

        for (std::uint16_t i = 0; i < stackCount; ++i)
        {
            const std::uint16_t row = i * sliceRingCount;
            for (std::uint16_t j = 0; j < sliceCount; ++j)
            {
                const uint16_t col = row + j;
                const uint16_t leftBottom = col;
                const uint16_t rightBottom = col + 1;
                const uint16_t leftTop = col + sliceRingCount;
                const uint16_t rightTop = leftTop + 1;

                indices.push_back(leftBottom);
                indices.push_back(leftTop);
                indices.push_back(rightTop);
                indices.push_back(leftBottom);
                indices.push_back(rightTop);
                indices.push_back(rightBottom);
            }
        }

        {
            positions.push_back(MakePosition(0.f, height, 0.f));
            texCoords.push_back(MakeTexCoord(0.5f, 0.5f));
            tangents.push_back(MakeDir(1.f, 0.f, 0.f));
            normals.push_back(MakeDir(0.f, 1.f, 0.f));

            // vertices.push_back(SimpleVertex{ topPos, topNormal, topTangentU, topUV });
            const auto topVertexPos = static_cast<std::uint16_t>(positions.size() - 1);

            float angle = 0.f;
            for (std::uint16_t i = 0; i <= sliceCount; ++i)
            {
                angle += angleStep;
                const auto x = topRadius * std::cos(angle);
                const auto z = topRadius * std::sin(angle);
                positions.push_back(MakePosition(x, height, z));
                normals.push_back(MakeDir(0.f, 1.f, 0.f));
                tangents.push_back(MakeDir(1.f, 0.f, 0.f));
                // What the fuck?
                texCoords.push_back({x / height, z / height});
            }

            for (std::uint16_t i = 1; i <= sliceCount; ++i)
            {
                indices.push_back(topVertexPos);
                indices.push_back(i + topVertexPos + 1);
                indices.push_back(i + topVertexPos);
            }
        }

        {
            const auto bottomPos = MakePosition(0.f, 0.f, 0.f);
            const auto bottomUV = MakeTexCoord(1.f, 1.f);
            const auto bottomTangentU = MakeDir(1.f, 0.f, 0.f);
            const auto bottomNormal = MakeDir(0.f, -1.f, 0.f);
            const auto bottomBitangentU = MakeDir(0.0f, 0.0f, -1.0f);
            positions.push_back(bottomPos);
            texCoords.push_back(bottomUV);
            tangents.push_back(bottomTangentU);
            normals.push_back(bottomNormal);

            const auto bottomVertexPos = static_cast<std::uint16_t>(positions.size() - 1);
            float angle = 0.f;
            for (std::uint16_t i = 0; i <= sliceCount; ++i)
            {
                angle += angleStep;
                const auto x = bottomRadius * std::cos(angle);
                const auto z = bottomRadius * std::sin(angle);
                const auto pos = MakePosition(x, 0.f, z);
                const auto normal = bottomNormal;
                const auto tangentU = bottomTangentU;
                const auto bitangentU = bottomBitangentU;
                // What the fuck?
                const auto uv = MakeTexCoord(x / height + 0.5f, z / height + 0.5f);
                positions.push_back(pos);
                normals.push_back(normal);
                tangents.push_back(tangentU);
                texCoords.push_back(uv);
            }

            for (std::uint16_t i = 1; i <= sliceCount; ++i)
            {
                indices.push_back(bottomVertexPos);
                indices.push_back(static_cast<std::uint16_t>(i + bottomVertexPos));
                indices.push_back(static_cast<std::uint16_t>(i + bottomVertexPos + 1));
            }
        }

        Ensures(positions.size() == verticesCount);
        Ensures(indices.size() == indicesCount);
        CheckStreamsSizeEqualityExceptColors(meshData);
    }

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount,
                      LoadedMesh& meshData)
    {
        using namespace DirectX;
        auto& [positions, normals, tangents, colors, texCoords, indices] = meshData;
        const std::uint16_t vertexCount = (sliceCount + 1) * (stackCount - 1) + 2;
        const std::uint16_t sliceRingCount = sliceCount + 1;

        meshData.Reserve(vertexCount);
        {
            const auto topPos = MakePosition(0.f, radius, 0.f);
            const auto topNormal = MakeDir(0.f, 1.f, 0.f);
            const auto topTangentU = MakeDir(1.f, 0.f, 0.f);
            const auto topUV = MakeTexCoord(0.f, 0.f);
            positions.push_back(topPos);
            normals.push_back(topNormal);
            tangents.push_back(topTangentU);
            texCoords.push_back(topUV);
        }
        const float yAngleStep = DirectX::XM_PI / stackCount;
        const float planeAngleStep = DirectX::XM_2PI / sliceCount;
        float phi = 0.f;
        // x(t) = rsin(phi)cos(theta)
        // y(t) = rcos(phi)
        // z(t) = rsin(phi)sin(theta)
        for (std::uint16_t i = 1; i < stackCount; ++i)
        {
            phi += yAngleStep;
            const float height = radius * std::cos(phi);
            const float currentStackRadius = radius * std::sin(phi);
            float theta = 0.f;
            for (std::uint16_t j = 0; j <= sliceCount; ++j)
            {
                theta += planeAngleStep;
                const auto pos = MakePosition(currentStackRadius * std::cos(theta), height,
                                              currentStackRadius * std::sin(theta));
                const auto uv = MakeTexCoord(theta / DirectX::XM_2PI, phi / DirectX::XM_PI);
                const auto normalU = XMVector3Normalize(Load(pos));
                VectorType normal = StoreVec(normalU);
                const auto tangentU = MakeDir(-std::sin(theta), 0.f, std::cos(theta));
                const auto tangenUVec = Load(tangentU);
                const auto biTangent = DirectX::XMVector3Cross(normalU, tangenUVec);

                positions.push_back(pos);
                normals.push_back(normal);
                tangents.push_back(tangentU);
                texCoords.push_back(uv);
            }
        }
        {
            const auto bottomPos = MakePosition(0.f, -radius, 0.f);
            const auto bottomNormal = MakeDir(0.f, -1.f, 0.f);
            const auto bottomUV = MakeTexCoord(0.f, 1.f);
            const auto bottomTangentU = MakeDir(1.f, 0.f, 0.f);
            const auto bottombiTangentU = MakeDir(0.0f, 0.0f, -1.0f);
            // SimpleVertex{ bottomPos, bottomNormal, bottomTangentU, bottomUV }
            positions.push_back(bottomPos);
            normals.push_back(bottomNormal);
            texCoords.push_back(bottomUV);
            tangents.push_back(bottomTangentU);
        }

        Expects(positions.size() < static_cast<std::size_t>(UINT16_MAX));
        Expects(positions.size() == vertexCount);
        indices.clear();
        const std::uint16_t indicesCount = (stackCount - 1) * sliceCount * 6;
        indices.reserve(indicesCount);
        for (std::uint16_t i = 1; i <= sliceCount; ++i)
        {
            indices.push_back(0);
            indices.push_back(static_cast<std::uint16_t>(i + 1));
            indices.push_back(i);
        }

        std::uint16_t baseIndex = 1;

        for (std::uint16_t i = 1; i < stackCount - 1; ++i)
        {
            const std::uint16_t row = i * sliceRingCount + baseIndex;
            for (std::uint16_t j = 0; j < sliceCount; ++j)
            {
                const uint16_t col = row + j;
                const uint16_t leftBottom = col;
                const uint16_t rightBottom = col + 1;
                const uint16_t leftTop = col - sliceRingCount;
                const uint16_t rightTop = leftTop + 1;

                indices.push_back(leftBottom);
                indices.push_back(leftTop);
                indices.push_back(rightTop);
                indices.push_back(leftBottom);
                indices.push_back(rightTop);
                indices.push_back(rightBottom);
            }
        }

        const auto bottomIndex = static_cast<std::uint16_t>(positions.size() - 1);
        baseIndex = bottomIndex - sliceRingCount;

        for (std::uint16_t i = 0; i < sliceCount; ++i)
        {
            indices.push_back(bottomIndex);
            indices.push_back(static_cast<std::uint16_t>(baseIndex + i));
            indices.push_back(static_cast<std::uint16_t>(baseIndex + i + 1));
        }
        Ensures(indices.size() == indicesCount);
        CheckStreamsSizeEqualityExceptColors(meshData);
    }

    void LoadedMesh::Reserve(std::size_t size)
    {
        Positions.reserve(size);
        Normals.reserve(size);
        Tangents.reserve(size);
        Colors.reserve(size);
        TexCoords.reserve(size);
    }

    void LoadedMesh::Clear()
    {
        Positions.clear();
        Normals.clear();
        Tangents.clear();
        Colors.clear();
        TexCoords.clear();
    }
} // namespace dx
