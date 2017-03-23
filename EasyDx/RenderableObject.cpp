#include "RenderableObject.hpp"
#include "SimpleVertex.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <utility>
#include <d3d11.h>

namespace dx
{
    RenderableObject::RenderableObject(gsl::span<Mesh> meshes)
        : meshes_{ meshes.begin(), meshes.end() }
    {
    }

    //TODO: ...
    void CheckAiReturn(aiReturn code)
    {
        ThrowIf<std::runtime_error>(code != aiReturn::aiReturn_SUCCESS, "Assimp error");
    }

    void AiColorToFloat3(const aiColor3D& color, DirectX::XMFLOAT3& float3) noexcept
    {
        float3.x = color.r;
        float3.y = color.g;
        float3.z = color.b;
    }

    void LoadAllMeshFromScene(ID3D11Device& device, const aiScene* scene, std::vector<Mesh>& meshes, gsl::span<const std::shared_ptr<Material>> materials)
    {
        const auto meshCount = scene->mNumMeshes;
        meshes.reserve(meshCount);
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
                const auto& pos = mesh->mVertices[i];
                const auto& normal = mesh->mNormals[i];
                const auto tex = mesh->mTextureCoords[j];

                SimpleVertex vertex = {
                    { pos.x, pos.y, pos.z },
                    { normal.x, normal.y, normal.z }
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
                    //.insert(indicesInMesh.end(), faceIndices.begin(), faceIndices.end());
                }
            }
            meshes.emplace_back(device,
                gsl::make_span(verticesInMesh),
                gsl::make_span(indicesInMesh),
                materials[mesh->mMaterialIndex]);
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

    void LoadAllMaterialsFromScene(ID3D11Device& device, const aiScene* scene, const fs::path& parentPath, std::vector<std::shared_ptr<Material>>& materials)
    {
        const auto materialsInScene = gsl::make_span(scene->mMaterials, scene->mNumMaterials);
        materials.clear();
        materials.reserve(materialsInScene.size());
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
            std::vector<wrl::ComPtr<ID3D11Texture2D>> textures;
            std::vector<wrl::ComPtr<ID3D11SamplerState>> samplerStates;
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
                auto texture = Texture::Load2DFromFile(device, parent.append(s2ws(texPath.C_Str()) ));
                D3D11_SAMPLER_DESC samplerDesc = {};
                //TODO: How to get this from assimp?
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                samplerDesc.AddressU = FromAssimpTexMapModeToDX(uMapMode);
                samplerDesc.AddressV = FromAssimpTexMapModeToDX(vMapMode);
                samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                wrl::ComPtr<ID3D11SamplerState> samplerState;
                TryHR(device.CreateSamplerState(&samplerDesc, samplerState.ReleaseAndGetAddressOf()));
                textures.push_back(std::move(texture));
                samplerStates.push_back(std::move(samplerState));
            }
            auto resultMat = std::make_shared<Material>();
            auto& smoothness = resultMat->Smooth;
            AiColorToFloat3(ambient, smoothness.Amibient);
            AiColorToFloat3(specular, smoothness.Specular);
            AiColorToFloat3(diffuse, smoothness.Diffuse);
            smoothness.SpecularPower = specularPower;
            if (!textures.empty())
            {
                resultMat->MainTexture = { Texture::GetView(device, Ref(textures.front())), samplerStates.front() };
            }

            materials.push_back(std::move(resultMat));
        }
    }

    RenderableObject RenderableObject::LoadFromModel(ID3D11Device& device, const fs::path& filePath)
    {
        const auto pathString = filePath.u8string();
        Assimp::Importer importer;

        const auto AssimpThrow = [&](auto ptr)
        {
            ThrowIf<std::runtime_error>(ptr == nullptr, importer.GetErrorString());
            return ptr;
        };
        const auto scene = AssimpThrow(importer.ReadFile(pathString.c_str(), aiProcessPreset_TargetRealtime_MaxQuality));
        
        std::vector<std::shared_ptr<Material>> materials;
        LoadAllMaterialsFromScene(device, scene, filePath.parent_path(), materials);
        std::vector<Mesh> meshes;
        LoadAllMeshFromScene(device, scene, meshes, gsl::make_span(materials));

        //TODO: wtf
        RenderableObject object;
        object.meshes_ = std::move(meshes);
        return object;
    }

    DirectX::XMMATRIX RenderableObject::ComputeWorld() const noexcept
    {
        using namespace DirectX;
        return
            XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
            XMMatrixRotationQuaternion(XMLoadFloat4(&Rotation)) *
            XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
    }

    void RenderableObject::Render(ID3D11DeviceContext& deviceContext)
    {
        for (auto& mesh : meshes_)
        {
            mesh.Render(deviceContext);
        }
    }

    gsl::span<Mesh> RenderableObject::GetMeshes() noexcept
    {
        return gsl::make_span(meshes_);
    }
}