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
}
