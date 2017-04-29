#include "Shaders.hpp"
#include "StlUtilities.hpp"
#include <gsl/gsl_assert>
#include <unordered_map>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, gsl::span<const gsl::byte> byteCode)
    {
        wrl::ComPtr<ID3D11VertexShader> vs;
        TryHR(device.CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, vs.GetAddressOf()));
        return vs;
    }

    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, gsl::span<const gsl::byte> byteCode)
    {
        wrl::ComPtr<ID3D11PixelShader> ps;
        TryHR(device.CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, ps.GetAddressOf()));
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
        gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc)
    {
        auto byteCode = dx::CompileShaderFromFile(filePath.c_str(), entryName, "vs_5_0");
        return FromByteCode(device, BlobToSpan(Ref(byteCode)), layoutDesc);
    }

    VertexShader VertexShader::FromByteCode(ID3D11Device& device, gsl::span<const gsl::byte> byteCode, gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc)
    {
        VertexShader vs;
        TryHR(device.CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, vs.shader_.ReleaseAndGetAddressOf()));
        TryHR(device.CreateInputLayout(layoutDesc.data(), layoutDesc.size(), byteCode.data(),
            byteCode.size(), vs.layout_.GetAddressOf()));
        return vs;
    }

    void VertexShader::Bind(ID3D11DeviceContext& deviceContext) const
    {
        deviceContext.IASetInputLayout(&GetLayout());
        deviceContext.VSSetShader(&GetShader(), nullptr, 0);
        const auto cbs = ComPtrsCast(gsl::make_span(ConstantBuffers));
        deviceContext.VSSetConstantBuffers(0, cbs.size(), cbs.data());
    }

    ID3D11VertexShader& VertexShader::GetShader() const
    {
        return Ref(shader_);
    }

    ID3D11InputLayout& VertexShader::GetLayout() const
    {
        return Ref(layout_);
    }

    VertexShaderView VertexShader::Get() const noexcept
    {
        return { shader_.Get(), layout_.Get() };
    }

    VertexShader::VertexShader(wrl::ComPtr<ID3D11VertexShader> shader, wrl::ComPtr<ID3D11InputLayout> layout)
        : shader_{std::move(shader)},
        layout_{std::move(layout)}
    {
    }

    PixelShader PixelShader::CompileFromFile(ID3D11Device& device, const fs::path& filePath, const char* entryName)
    {
        auto psByteCode = CompileShaderFromFile(
            filePath.c_str(),
            entryName,
            "ps_5_0"
        );
        return FromByteCode(device, BlobToSpan(Ref(psByteCode)));
    }

    PixelShader PixelShader::FromByteCode(ID3D11Device& device, gsl::span<const gsl::byte> byteCode)
    {
        return { CreatePixelShader(device, byteCode) };
    }

    void PixelShader::Bind(ID3D11DeviceContext& deviceContext) const
    {
        const auto cbs = ComPtrsCast(gsl::make_span(ConstantBuffers));
        deviceContext.PSSetConstantBuffers(0, cbs.size(), cbs.data());
        deviceContext.PSSetShader(&GetShader(), nullptr, 0);
    }

    ID3D11PixelShader& PixelShader::GetShader() const noexcept
    {
        return Ref(shader_);
    }

    PixelShaderView PixelShader::Get() const noexcept
    {
        return {
            shader_.Get(),
            ComPtrsCast(gsl::make_span(ConstantBuffers))
        };
    }

    PixelShader::PixelShader(wrl::ComPtr<ID3D11PixelShader> shader)
        : shader_{std::move(shader)}
    {
    }

    /*std::unordered_map<std::uint32_t, VertexShader> VertexShaders;
    std::unordered_map<std::uint32_t, wrl::ComPtr<ID3D11PixelShader>> PixelShaders;

    const VertexShader* GetVertexShader(std::uint32_t index)
    {
        return stlext::FindOr(VertexShaders, index, {}, [](const VertexShader& shader) { return &shader; });
    }

    ID3D11PixelShader* GetPixelShader(std::uint32_t index)
    {
        return stlext::FindOr(PixelShaders, index, {}, [](const wrl::ComPtr<ID3D11PixelShader>& shader) { return shader.Get();  });
    }

    void RegisterVertexShader(std::uint32_t index, VertexShader vertexShader)
    {
        VertexShaders.insert({ index, std::move(vertexShader) });
    }

    void RegisterPixelShader(std::uint32_t index, wrl::ComPtr<ID3D11PixelShader> pixelShader)
    {
        PixelShaders.insert({ index, std::move(pixelShader) });
    }*/
}
