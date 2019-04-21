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

    struct Rect
    {
        float Left;
        float Top;
        float Right;
        float Bottom;
    };

    DirectX::XMFLOAT4 MakePosition4(const DirectX::XMFLOAT3& position) noexcept;
    DirectX::XMFLOAT4 MakePosition4(float x, float y, float z) noexcept;
    DirectX::XMFLOAT4 MakePosition4(DirectX::XMVECTOR vec) noexcept;
    DirectX::XMFLOAT4
    MakeDirection4(const DirectX::XMFLOAT3& direction) noexcept;
    DirectX::XMFLOAT4 MakeDirection4(float x, float y, float z) noexcept;
    DirectX::XMFLOAT4 MakeDirection4(DirectX::XMVECTOR vec) noexcept;

    void Normalize(DirectX::XMFLOAT3& vec) noexcept;
    void Normalize(DirectX::XMFLOAT4& vec) noexcept;
} // namespace dx