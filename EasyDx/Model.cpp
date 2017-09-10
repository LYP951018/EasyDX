#include "pch.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Shaders.hpp"
#include "GameObject.hpp"
#include "SimpleVertex.hpp"
#include "Renderable.hpp"
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

    void AiColorToFloat4(const aiColor3D& color, DirectX::XMFLOAT4& float4) noexcept
    {
        float4.x = color.r;
        float4.y = color.g;
        float4.z = color.b;
        float4.w = 1.f;
    }

    void LoadAllObjectsFromScene(const aiScene* scene,
        std::vector<std::shared_ptr<GameObject>>& objects,
        gsl::span<const std::shared_ptr<Smoothness>> smoothnesses,
        gsl::span<const std::shared_ptr<Texture>> textures)
    {
        const auto meshCount = scene->mNumMeshes;
        objects.reserve(meshCount);
        std::vector<SimpleVertex> verticesInMesh;
        std::vector<std::uint16_t> indicesInMesh;
        for (unsigned j = 0; j < meshCount; ++j)
        {
            const auto mesh = scene->mMeshes[j];
            verticesInMesh.clear();
            indicesInMesh.clear();
            const auto verticesNum = mesh->mNumVertices;
            verticesInMesh.reserve(verticesNum);
            for (unsigned i = 0; i < verticesNum; ++i)
            {
                auto object = MakeShared<GameObject>();

                const auto& pos = mesh->mVertices[i];
                const auto& normal = mesh->mNormals[i];
                const auto& tangent = mesh->mTangents[i];
                const auto tex = mesh->mTextureCoords[j];

                SimpleVertex vertex = {
                    { pos.x, pos.y, pos.z },
                    { normal.x, normal.y, normal.z },
                    { tangent.x, tangent.y, tangent.z }
                };

                if (tex == nullptr)
                {
                    vertex.TexCoord = { 1.f, 1.f };
                }
                else
                {
                    vertex.TexCoord = { tex[i].x, tex[i].y };
                }
                verticesInMesh.push_back(vertex);

                const auto faces = gsl::make_span(mesh->mFaces, mesh->mNumFaces);

                indicesInMesh.reserve(faces.size() * 3);
                for (const auto& face : faces)
                {
                    const auto faceIndices = gsl::make_span(face.mIndices, face.mNumIndices);
                    if (faceIndices.size() != 3)
                        continue;

                    for (auto index : faceIndices)
                    {
                        indicesInMesh.push_back(static_cast<std::uint16_t>(index));
                    }
                }
                object->AddComponent(MakeShared<SimpleCpuMesh>(std::move(verticesInMesh), std::move(indicesInMesh)));
                object->AddComponent(std::move(smoothnesses[mesh->mMaterialIndex]));
                object->AddComponent(std::move(textures[mesh->mMaterialIndex]));
                objects.push_back(std::move(object));
            }
        }
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
            throw std::runtime_error{ "Unsupported map mode." };
            break;
        }
    }

    void LoadAllMaterialsFromScene(ID3D11Device& device, const aiScene* scene, 
        const fs::path& parentPath, 
        std::vector<std::shared_ptr<Smoothness>>& smoothnesses,
        std::vector<std::shared_ptr<Texture>>& textures)
    {
        const auto materialsInScene = gsl::make_span(scene->mMaterials, scene->mNumMaterials);
        smoothnesses.clear();
        smoothnesses.reserve(materialsInScene.size());
        for (auto material : materialsInScene)
        {
            aiColor3D diffuse, specular, ambient;
            CheckAiReturn(material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse));
            CheckAiReturn(material->Get(AI_MATKEY_COLOR_SPECULAR, specular));
            CheckAiReturn(material->Get(AI_MATKEY_COLOR_AMBIENT, ambient));
            float specularPower;
            CheckAiReturn(material->Get(AI_MATKEY_SHININESS, specularPower));
            constexpr auto kTexType = aiTextureType::aiTextureType_DIFFUSE;
            const auto texCount = material->GetTextureCount(kTexType);
            Ensures(texCount <= 1); //only single texture is supported
            for (unsigned i = 0; i < texCount; ++i)
            {
                aiString texPath;
                CheckAiReturn(material->Get(AI_MATKEY_TEXTURE(kTexType, i), texPath));
                aiTextureMapMode uMapMode = aiTextureMapMode_Wrap, vMapMode = aiTextureMapMode_Wrap;
                //FIXME
                material->Get(AI_MATKEY_MAPPINGMODE_U(kTexType, i), uMapMode);
                material->Get(AI_MATKEY_MAPPINGMODE_V(kTexType, i), vMapMode);
                /*CheckAiReturn(material->Get(AI_MATKEY_MAPPINGMODE_U(kTexType, i), uMapMode));
                CheckAiReturn(material->Get(AI_MATKEY_MAPPINGMODE_V(kTexType, i), vMapMode));*/
                auto parent = parentPath;
                auto texture = Load2DTexFromFile(device, parent.append(s2ws(texPath.C_Str())));
                D3D11_SAMPLER_DESC samplerDesc = {};
                //TODO: How to get this from assimp?
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                samplerDesc.AddressU = FromAssimpTexMapModeToDX(uMapMode);
                samplerDesc.AddressV = FromAssimpTexMapModeToDX(vMapMode);
                samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                wrl::ComPtr<ID3D11SamplerState> samplerState;
                TryHR(device.CreateSamplerState(&samplerDesc, samplerState.ReleaseAndGetAddressOf()));
                textures.push_back(MakeShared<Texture>(device, std::move(texture), std::move(samplerState)));
            }
            Smoothness smoothness;
            AiColorToFloat4(ambient, smoothness.Amibient);
            AiColorToFloat4(specular, smoothness.Specular);
            AiColorToFloat4(diffuse, smoothness.Diffuse);
            smoothness.SpecularPower = specularPower;
            smoothnesses.push_back(MakeShared<Smoothness>(smoothness));
        }
    }

    void LoadFromModel(ID3D11Device& device, const fs::path& filePath,
        std::vector<std::shared_ptr<GameObject>>& objects)
    {
        const auto pathString = filePath.u8string();
        Assimp::Importer importer;

        const auto AssimpThrow = [&](auto ptr)
        {
            ThrowIf<std::runtime_error>(ptr == nullptr, importer.GetErrorString());
            return ptr;
        };
        const auto scene = AssimpThrow(importer.ReadFile(pathString.c_str(), aiProcessPreset_TargetRealtime_MaxQuality));
        std::vector<std::shared_ptr<Smoothness>> materials;
        std::vector<std::shared_ptr<Texture>> textures;
        LoadAllMaterialsFromScene(device, scene, filePath.parent_path(), materials, textures);
        objects.clear();
        LoadAllObjectsFromScene(scene, objects, gsl::make_span(materials), gsl::make_span(textures));
    }

    void MakeCylinder(float bottomRadius, float topRadius, float height,
        std::uint16_t sliceCount, std::uint16_t stackCount, CpuMesh<SimpleVertex>& meshData)
    {
        using namespace DirectX;
        auto& vertices = meshData.Vertices;
        auto& indices = meshData.Indices;
        vertices.clear();
        const std::uint16_t sliceRingCount = sliceCount + 1;
        const std::uint16_t stackRingCount = stackCount + 1;
        const std::uint16_t verticesCount = (stackRingCount + 2) * sliceRingCount + 2;

        vertices.reserve(verticesCount);
        const float heightPerStack = height / stackCount;
        const float angleStep = DirectX::XM_2PI / sliceCount;
        const auto radiusDelta = bottomRadius - topRadius;
        const float radiusStep = radiusDelta / stackCount;
        float currentHeight = 0.f;
        float radius = bottomRadius;
        //r(v) = bottomRadius - (bottomRadius - topRadius) * v
        //v = y / h
        //y(v) = h * v
        //x(t) = r(v) * cos(t)
        //y(t) = v
        //z(t) = r(v) * sin(t)
        //dx/dt = -r(v) * sin(t)
        //dy/dt = 0
        //dx/dv = (b - top) * cos(t)
        //dy/dv = h
        //dz/dv = (b - top) * sin(t)
        for (std::uint16_t i = 0; i < stackRingCount; ++i)
        {
            float angle = 0.f;
            for (std::uint16_t j = 0; j <= sliceCount; ++j)
            {
                const auto c = std::cos(angle);
                const auto s = std::sin(angle);
                const auto pos = XMFLOAT3{ radius * c, currentHeight,  radius * s };
                const auto uv = XMFLOAT2{ 1.f - currentHeight / height, angle / DirectX::XM_2PI };
                //unit length already.
                const auto tangentU = XMFLOAT3{ -s, 0, c };
                const auto biTangent = XMFLOAT3{ radiusDelta * c, -height, radiusDelta * s };
                const auto tangentUVec = XMLoadFloat3(&tangentU);
                const auto biTangentVec = XMLoadFloat3(&biTangent);
                const auto normalUVec = XMVector3Normalize(XMVector3Cross(tangentUVec, biTangentVec));
                XMFLOAT3 normalU;
                XMStoreFloat3(&normalU, normalUVec);
                vertices.push_back(SimpleVertex{ pos, normalU, tangentU, uv });
                angle += angleStep;
            }
            currentHeight += heightPerStack;
            radius -= radiusStep;
        }

        Expects(vertices.size() < static_cast<std::size_t>(UINT16_MAX));
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
            const auto topPos = DirectX::XMFLOAT3{ 0.f, height, 0.f };
            const auto topUV = DirectX::XMFLOAT2{ 0.5f, 0.5f };
            const auto topTangentU = DirectX::XMFLOAT3{ 1.f, 0.f, 0.f };
            const auto topNormal = DirectX::XMFLOAT3{ 0.f, 1.f, 0.f };
            vertices.push_back(SimpleVertex{ topPos, topNormal, topTangentU, topUV });
            const auto topVertexPos = static_cast<std::uint16_t>(vertices.size() - 1);

            float angle = 0.f;
            for (std::uint16_t i = 0; i <= sliceCount; ++i)
            {
                angle += angleStep;
                const auto x = topRadius * std::cos(angle);
                const auto z = topRadius * std::sin(angle);
                const auto pos = XMFLOAT3{ x, height, z };
                const auto normal = XMFLOAT3{ 0.f, 1.f, 0.f };
                const auto tangentU = XMFLOAT3{ 1.f, 0.f, 0.f };
                //What the fuck?
                const auto uv = DirectX::XMFLOAT2{ x / height, z / height };
                vertices.push_back(SimpleVertex{ pos, normal, tangentU, uv });
            }

            for (std::uint16_t i = 1; i <= sliceCount; ++i)
            {
                indices.push_back(topVertexPos);
                indices.push_back(i + topVertexPos + 1);
                indices.push_back(i + topVertexPos);
            }
        }

        {
            const auto bottomPos = XMFLOAT3{ 0.f, 0.f, 0.f };
            const auto bottomUV = XMFLOAT2{ 1.f, 1.f };
            const auto bottomTangentU = DirectX::XMFLOAT3{ 1.f, 0.f, 0.f };
            const auto bottomNormal = DirectX::XMFLOAT3{ 0.f, -1.f, 0.f };
            vertices.push_back(SimpleVertex{ bottomPos, bottomNormal, bottomTangentU, bottomUV });
            const auto bottomVertexPos = static_cast<std::uint16_t>(vertices.size() - 1);
            float angle = 0.f;
            for (std::uint16_t i = 0; i <= sliceCount; ++i)
            {
                angle += angleStep;
                const auto x = bottomRadius * std::cos(angle);
                const auto z = bottomRadius * std::sin(angle);
                const auto pos = DirectX::XMFLOAT3{ x, 0.f, z };
                const auto normal = bottomNormal;
                const auto tangentU = bottomTangentU;
                //What the fuck?
                const auto uv = DirectX::XMFLOAT2{ x / height + 0.5f, z / height + 0.5f };
                vertices.push_back(SimpleVertex{ pos, normal, tangentU, uv });
            }

            for (std::uint16_t i = 1; i <= sliceCount; ++i)
            {
                indices.push_back(bottomVertexPos);
                indices.push_back(static_cast<std::uint16_t>(i + bottomVertexPos));
                indices.push_back(static_cast<std::uint16_t>(i + bottomVertexPos + 1));
            }
        }

        Ensures(vertices.size() == verticesCount);
        Ensures(indices.size() == indicesCount);
    }

    void MakeUVSphere(float radius, std::uint16_t sliceCount, std::uint16_t stackCount,
        CpuMesh<SimpleVertex>& meshData)
    {
        using namespace DirectX;
        auto& vertices = meshData.Vertices;
        auto& indices = meshData.Indices;
        vertices.clear();
        const std::uint16_t vertexCount = (sliceCount + 1) * (stackCount - 1) + 2;
        const std::uint16_t sliceRingCount = sliceCount + 1;

        vertices.reserve(vertexCount);
        {
            const auto topPos = XMFLOAT3{ 0.f, radius, 0.f };
            const auto topNormal = XMFLOAT3{ 0.f, 1.f, 0.f };
            const auto topTangentU = XMFLOAT3{ 1.f, 0.f, 0.f };
            const auto topUV = XMFLOAT2{ 0.f, 0.f };
            vertices.push_back(SimpleVertex{ topPos, topNormal, topTangentU, topUV });
        }
        const float yAngleStep = DirectX::XM_PI / stackCount;
        const float planeAngleStep = DirectX::XM_2PI / sliceCount;
        float phi = 0.f;
        //x(t) = rsin(phi)cos(theta)
        //y(t) = rcos(phi)
        //z(t) = rsin(phi)sin(theta)
        for (std::uint16_t i = 1; i < stackCount; ++i)
        {
            phi += yAngleStep;
            const float height = radius * std::cos(phi);
            const float currentStackRadius = radius * std::sin(phi);
            float theta = 0.f;
            for (std::uint16_t j = 0; j <= sliceCount; ++j)
            {
                theta += planeAngleStep;
                const auto pos = XMFLOAT3{ currentStackRadius * std::cos(theta), height, currentStackRadius * std::sin(theta) };
                const auto uv = XMFLOAT2{ theta / DirectX::XM_2PI, phi / DirectX::XM_PI };
                XMFLOAT3 normal;
                XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));
                const auto tangentU = XMFLOAT3{ -currentStackRadius * std::sin(theta), 0.f, currentStackRadius * std::cos(theta) };
                vertices.push_back(SimpleVertex{ pos, normal, tangentU, uv });
            }
        }
        {
            const auto bottomPos = DirectX::XMFLOAT3{ 0.f, -radius, 0.f };
            const auto bottomNormal = DirectX::XMFLOAT3{ 0.f, -1.f, 0.f };
            const auto bottomUV = DirectX::XMFLOAT2{ 0.f, 1.f };
            const auto bottomTangentU = XMFLOAT3{ 1.f, 0.f, 0.f };
            vertices.push_back(SimpleVertex{ bottomPos, bottomNormal, bottomTangentU, bottomUV });
        }

        Expects(vertices.size() < static_cast<std::size_t>(UINT16_MAX));
        Expects(vertices.size() == vertexCount);
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

        const auto bottomIndex = static_cast<std::uint16_t>(vertices.size() - 1);
        baseIndex = bottomIndex - sliceRingCount;

        for (std::uint16_t i = 0; i < sliceCount; ++i)
        {
            indices.push_back(bottomIndex);
            indices.push_back(static_cast<std::uint16_t>(baseIndex + i));
            indices.push_back(static_cast<std::uint16_t>(baseIndex + i + 1));
        }
        Ensures(indices.size() == indicesCount);
    }
}
