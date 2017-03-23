#pragma once

#include "Mesh.hpp"
#include "Common.hpp"
#include <vector>
#include <gsl/span>
#include <DirectXMath.h>

namespace dx
{
    class RenderableObject
    {
    public:
        //是否应该提供默认构造函数？
        RenderableObject() = default;
        RenderableObject(gsl::span<Mesh> meshes);

        RenderableObject(const RenderableObject&) = delete;
        RenderableObject& operator= (const RenderableObject&) = delete;

        RenderableObject(RenderableObject&&) = default;
        RenderableObject& operator= (RenderableObject&&) = default;

        static RenderableObject LoadFromModel(ID3D11Device& device, const fs::path& filePath);

        DirectX::XMFLOAT3 Scale{ 1.f, 1.f, 1.f };
        DirectX::XMFLOAT4 Rotation{ 0.f, 0.f, 0.f, 1.f };
        DirectX::XMFLOAT3 Translation{ 0.f, 0.f, 0.f };

        DirectX::XMMATRIX ComputeWorld() const noexcept;
        void Render(ID3D11DeviceContext& deviceContext);
        gsl::span<Mesh> GetMeshes() noexcept;

    private:
        std::vector<Mesh> meshes_;
    };
}