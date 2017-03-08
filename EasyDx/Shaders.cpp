#include "Shaders.hpp"
#include "DXHelpers.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>

namespace dx
{
    VertexShader VertexShader::CompileFromFile(ID3D11Device& device,
        const fs::path& filePath,
        const char* entryName,
        gsl::span<D3D11_INPUT_ELEMENT_DESC> layoutDesc)
    {
        auto byteCode = dx::CompileShaderFromFile(filePath.c_str(), entryName, "vs_5_0");
        auto shader = CreateVertexShader(device, *byteCode.Get());
        wrl::ComPtr<ID3D11InputLayout> vertexLayout;
        TryHR(device.CreateInputLayout(layoutDesc.data(), layoutDesc.size(), byteCode->GetBufferPointer(),
            byteCode->GetBufferSize(), vertexLayout.GetAddressOf()));
        return {
            std::move(shader),
            std::move(vertexLayout)
        };
    }

    void VertexShader::Bind(ID3D11DeviceContext& deviceContext)
    {
        Ensures(shader_ != nullptr);
        deviceContext.IASetInputLayout(layout_.Get());
        deviceContext.VSSetShader(shader_.Get(), nullptr, 0);
    }

    VertexShader::VertexShader(wrl::ComPtr<ID3D11VertexShader> shader, wrl::ComPtr<ID3D11InputLayout> layout)
        : shader_{std::move(shader)},
        layout_{std::move(layout)}
    {
    }

    wrl::ComPtr<ID3D11PixelShader> PixelShader::CompileFromFile(ID3D11Device& device, const fs::path& filePath, const char* entryName)
    {
        auto psByteCode = CompileShaderFromFile(
            filePath.c_str(),
            entryName,
            "ps_5_0"
        );
        return dx::CreatePixelShader(device, *psByteCode.Get());
    }
}
