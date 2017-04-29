#pragma once

#include "Mesh.hpp"
#include "Common.hpp"
#include "Transformation.hpp"
#include "Mesh.hpp"
#include <vector>
#include <gsl/span>
#include <DirectXMath.h>

namespace dx
{
    struct Object
    {
        Transformation Transform;
        std::vector<Mesh> Meshes;

        template<typename Fn>
        void Render(ID3D11DeviceContext& context, Fn beforeRendering)
        {
            for (const auto& mesh : Meshes)
            {
                beforeRendering(mesh);
                mesh.DrawIndexed(context);
            }
        }
    };

    class StillObject
    {
    public:
        std::vector<Mesh> Meshes;
        void SetWorld(const DirectX::XMFLOAT4X3& world) noexcept;
        DirectX::XMMATRIX GetWorld() const noexcept;
        void Render(ID3D11DeviceContext& context) const;

    private:
        DirectX::XMFLOAT4X3 worldMatrix_;
    };

    class Instances
    {
    public:
        std::vector<Mesh> Meshes;
        VertexBuffer InstanceBuffer;
        std::uint32_t InstanceCount;
        void Render(ID3D11DeviceContext& context) const;
    };
}