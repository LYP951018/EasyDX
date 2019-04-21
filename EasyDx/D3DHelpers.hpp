#pragma once

namespace dx
{
    void SetName(ID3D11DeviceChild& child, std::string_view name);
    wrl::ComPtr<ID3D11RenderTargetView>
    MakeRtFromTex2D(ID3D11Device& device3D, ID3D11Texture2D& texture2D);
} // namespace dx