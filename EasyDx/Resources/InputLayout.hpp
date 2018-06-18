#pragma once

namespace dx
{
    void Bind(ID3D11DeviceContext& context3D, ::ID3D11InputLayout& layout);

    wrl::ComPtr<ID3D11InputLayout> MakeLayout(ID3D11Device& device, gsl::span<const D3D11_INPUT_ELEMENT_DESC> desc,
        gsl::span<const std::byte> byteCode);
}