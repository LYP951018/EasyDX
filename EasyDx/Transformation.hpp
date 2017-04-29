#pragma once

#include <DirectXMath.h>

namespace dx
{
    struct Transformation
    {
        DirectX::XMFLOAT3 Scale;
        DirectX::XMFLOAT4 Rotation;
        DirectX::XMFLOAT3 Translation;

        Transformation() noexcept;
    };

    DirectX::XMMATRIX ComputeWorld(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotation,
        const DirectX::XMFLOAT3& translation) noexcept;
    DirectX::XMMATRIX ComputeWorld(const Transformation& transformation) noexcept;
}