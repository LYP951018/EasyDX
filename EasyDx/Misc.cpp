#include "pch.hpp"
#include "Misc.hpp"

namespace dx
{
    DirectX::XMFLOAT4 MakePosition(const DirectX::XMFLOAT3& position) noexcept
    {
        return { position.x, position.y, position.z, 1.f };
    }

    DirectX::XMFLOAT4 MakeDirection(const DirectX::XMFLOAT3& direction) noexcept
    {
        return { direction.x, direction.y, direction.z, 0.f };
    }

    DirectX::XMVECTOR MakePosVec(const DirectX::XMFLOAT3 & position) noexcept
    {
        return DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f);
    }

    DirectX::XMVECTOR MakeDirVec(const DirectX::XMFLOAT3 & direction) noexcept
    {
        return DirectX::XMVectorSet(direction.x, direction.y, direction.z, 0.0f);
    }
}
