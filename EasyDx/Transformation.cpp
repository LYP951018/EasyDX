#include "Transformation.hpp"

namespace dx
{
    Transformation::Transformation() noexcept
        : Scale{1.f, 1.f, 1.f},
        Rotation{0.f, 0.f, 0.f, 1.f},
        Translation{0.f, 0.f, 0.f}
    {
    }

    DirectX::XMMATRIX ComputeWorld(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& translation) noexcept
    {
        using namespace DirectX;
        return
            XMMatrixScaling(scale.x, scale.y, scale.z) *
            XMMatrixRotationQuaternion(XMLoadFloat4(&rotation)) *
            XMMatrixTranslation(translation.x, translation.y, translation.z);
    }

    DirectX::XMMATRIX ComputeWorld(const Transformation& transformation) noexcept
    {
        return ComputeWorld(transformation.Scale, transformation.Rotation, transformation.Translation);
    }
}


