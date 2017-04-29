#include "Objects.hpp"
#include "SimpleVertex.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include <utility>
#include <d3d11.h>

namespace dx
{
    void StillObject::SetWorld(const DirectX::XMFLOAT4X3& world) noexcept
    {
        worldMatrix_ = world;
    }

    DirectX::XMMATRIX StillObject::GetWorld() const noexcept
    {
        return DirectX::XMLoadFloat4x3(&worldMatrix_);
    }

    void StillObject::Render(ID3D11DeviceContext& context) const
    {
        for (const auto& mesh : Meshes)
        {
            mesh.DrawIndexed(context);
        }
    }

    void Instances::Render(ID3D11DeviceContext& context) const
    {
        for (const auto& mesh : Meshes)
        {
            mesh.DrawIndexedInstanced(context, InstanceBuffer, InstanceCount);
        }
    }
}