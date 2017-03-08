#include "Model.hpp"
#include "SimpleVertex.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gsl/span>
//由于 WRL::ComPtr 对于 incomplete type 的支持问题故这里需要 include d3d11.h。
//TODO: 换掉 ComPtr。
#include <d3d11.h>

namespace dx
{
    void LoadFromFile(ID3D11Device& device, const fs::path& filePath, std::vector<Mesh>& meshes)
    {
        meshes.clear();

        const auto pathString = filePath.u8string();
        const auto scene = aiImportFile(pathString.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

        const auto meshesInScene = gsl::make_span(scene->mMeshes, scene->mNumMeshes);
        meshes.reserve(meshesInScene.size());
        for (auto mesh : meshesInScene)
        {
            std::vector<SimpleVertex> verticesInMesh;
            std::vector<std::uint16_t> indicesInMesh;
            const auto verticesNum = mesh->mNumVertices;
            verticesInMesh.reserve(verticesNum);

            for (unsigned i = 0; i < verticesNum; ++i)
            {
                const auto& pos = mesh->mVertices[i];
                const auto& normal = mesh->mNormals[i];
                const auto tex = mesh->mTextureCoords[i];

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
                    vertex.TexCoord = { tex->x, tex->y };
                }
                verticesInMesh.push_back(vertex);

                const auto faces = gsl::make_span(mesh->mFaces, mesh->mNumFaces);
                for (const auto& face : faces)
                {
                    const auto faceIndices = gsl::make_span(face.mIndices, face.mNumIndices);
                    if (faceIndices.size() != 3)
                        continue;

                    indicesInMesh.reserve(faceIndices.size());
                    for (auto index : faceIndices)
                    {
                        indicesInMesh.push_back(static_cast<std::uint16_t>(index));
                    }
                    //.insert(indicesInMesh.end(), faceIndices.begin(), faceIndices.end());
                }
            }

            meshes.push_back(Mesh{
                device,
                gsl::make_span(verticesInMesh),
                gsl::make_span(indicesInMesh)
            });
        }
    }
}