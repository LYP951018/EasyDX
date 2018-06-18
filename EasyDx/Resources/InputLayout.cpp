#include "../pch.hpp"
#include "InputLayout.hpp"
#include <d3d11.h>

namespace dx
{
    void Bind(ID3D11DeviceContext& context3D, ::ID3D11InputLayout& layout)
    {
        context3D.IASetInputLayout(&layout);
    }

    wrl::ComPtr<ID3D11InputLayout> MakeLayout(ID3D11Device & device, gsl::span<const D3D11_INPUT_ELEMENT_DESC> desc, gsl::span<const std::byte> byteCode)
    {
        wrl::ComPtr<ID3D11InputLayout> layout;
        TryHR(device.CreateInputLayout(desc.data(), desc.size(), byteCode.data(), byteCode.size(), layout.GetAddressOf()));
        return layout;
    }
}