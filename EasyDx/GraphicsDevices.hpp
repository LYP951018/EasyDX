#pragma once

namespace dx
{
    using DeviceContext3DPair =
        std::pair<wrl::ComPtr<ID3D11Device>, wrl::ComPtr<ID3D11DeviceContext>>;

    DeviceContext3DPair MakeDevice3D();
} // namespace dx