#include "pch.hpp"
#include "DxMathWrappers.hpp"

namespace dx
{
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT3& f3) { return DirectX::XMLoadFloat3(&f3); }
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT4& f4) { return DirectX::XMLoadFloat4(&f4); }
    DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A& f3a) { return DirectX::XMLoadFloat3A(&f3a); }
}

