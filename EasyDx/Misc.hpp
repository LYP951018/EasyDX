#pragma once

#include <DirectXMath.h>

namespace dx
{
    struct Point
    {
        std::int32_t X, Y;
    };

    struct Size
    {
        std::uint32_t Width, Height;

        float GetAspectRatio() const noexcept
        {
            return static_cast<float>(Width) / static_cast<float>(Height);
        }
    };

    DirectX::XMFLOAT4 MakePosition(const DirectX::XMFLOAT3& position) noexcept;
    DirectX::XMFLOAT4 MakeDirection(const DirectX::XMFLOAT3& direction) noexcept;

    DirectX::XMVECTOR MakePosVec(const DirectX::XMFLOAT3& position) noexcept;
    DirectX::XMVECTOR MakeDirVec(const DirectX::XMFLOAT3& direction) noexcept;
}