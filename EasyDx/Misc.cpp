#include "pch.hpp"
#include "Misc.hpp"

namespace dx
{
    DirectX::XMFLOAT4 MakePosition4(const DirectX::XMFLOAT3& position) noexcept
    {
        return MakePosition4(position.x, position.y, position.z);
    }

    DirectX::XMFLOAT4 MakePosition4(float x, float y, float z) noexcept
    {
        return {x, y, z, 1.f};
    }

    DirectX::XMFLOAT4 MakePosition4(DirectX::XMVECTOR vec) noexcept
    {
        DirectX::XMFLOAT4 result;
        DirectX::XMStoreFloat4(&result, vec);
        result.w = 1.0f;
        return result;
    }

    DirectX::XMFLOAT4
    MakeDirection4(const DirectX::XMFLOAT3& direction) noexcept
    {
        return MakeDirection4(direction.x, direction.y, direction.z);
    }

    DirectX::XMFLOAT4 MakeDirection4(float x, float y, float z) noexcept
    {
        return {x, y, z, 0.f};
    }

    DirectX::XMFLOAT4 MakeDirection4(DirectX::XMVECTOR vec) noexcept
    {
        DirectX::XMFLOAT4 result;
        DirectX::XMStoreFloat4(&result, vec);
        result.w = 0.0f;
        return result;
    }

    void Normalize(DirectX::XMFLOAT3& vec) noexcept
    {
        const auto tmp = DirectX::XMLoadFloat3(&vec);
        const auto normalized = DirectX::XMVector3Normalize(tmp);
        DirectX::XMStoreFloat3(&vec, normalized);
    }

    void Normalize(DirectX::XMFLOAT4& vec) noexcept
    {
        const auto tmp = DirectX::XMLoadFloat4(&vec);
        const auto normalized = DirectX::XMVector3Normalize(tmp);
        DirectX::XMStoreFloat4(&vec, normalized);
    }
} // namespace dx
