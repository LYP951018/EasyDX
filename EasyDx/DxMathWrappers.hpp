#pragma once

#include <DirectXMath.h>

namespace dx
{
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT3& f3);
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT4& f4);
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A& f3a);

    template<typename T>
    T Store(DirectX::XMVECTOR vec)
    {
        T result;
        if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
        {
            DirectX::XMStoreFloat3(&result, vec);
        }
        else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
        {
            DirectX::XMStoreFloat4(&result, vec);
        }
        else if constexpr (std::is_same_v<T, DirectX::XMFLOAT3A>)
        {
            DirectX::XMStoreFloat3A(&result, vec);
        }
        return result;
    }
} // namespace dx

namespace DirectX
{
    inline bool operator==(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        return XMVector3NearEqual(::dx::Load(lhs), ::dx::Load(rhs),
                                  XMVectorSplatEpsilon());
    }

    inline bool operator!=(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        return !(lhs == rhs);
    }
} // namespace DirectX