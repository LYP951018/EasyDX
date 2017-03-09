#include "Shaders.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device & device, ID3D10Blob & blob)
    {
        wrl::ComPtr<ID3D11VertexShader> vs;
        TryHR(device.CreateVertexShader(blob.GetBufferPointer(), blob.GetBufferSize(), nullptr, vs.GetAddressOf()));
        return vs;
    }

    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device & device, ID3D10Blob & blob)
    {
        wrl::ComPtr<ID3D11PixelShader> ps;
        TryHR(device.CreatePixelShader(blob.GetBufferPointer(), blob.GetBufferSize(), nullptr, ps.GetAddressOf()));
        return ps;
    }

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(const wchar_t* fileName, const char* entryPoint, const char* shaderModel)
    {
        std::uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        wrl::ComPtr<ID3DBlob> shaderBlob;
        wrl::ComPtr<ID3DBlob> errorBlob;
        const auto hr = D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, shaderModel,
            compileFlags, {}, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            using namespace std::string_literals;
            const auto errorMessage =
                "Compile error in shader " + ws2s(fileName) + ", " + static_cast<const char*>(errorBlob->GetBufferPointer());
            throw std::runtime_error{ errorMessage };
        }
        return shaderBlob;
    }

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
