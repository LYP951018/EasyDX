#include "pch.hpp"
#include "Shaders.hpp"
#include "Game.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, gsl::span<const std::byte> byteCode)
    {
        wrl::ComPtr<ID3D11VertexShader> vs;
        TryHR(device.CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, vs.GetAddressOf()));
        return vs;
    }

    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, gsl::span<const std::byte> byteCode)
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
            const auto errorMessage =
                "Compile error in shader " + ws2s(fileName) + ", " + static_cast<const char*>(errorBlob->GetBufferPointer());
            throw std::runtime_error{ errorMessage };
        }
        return shaderBlob;
    }

    void SetupShader(ID3D11DeviceContext& context3D, VertexShaderView vs)
    {
        context3D.IASetInputLayout(vs.Layout);
        context3D.VSSetShader(vs.Shader, nullptr, 0);
        const auto cbs = vs.Cbs;
        context3D.VSSetConstantBuffers(0, static_cast<UINT>(cbs.size()), cbs.data());
    }

    void SetupShader(ID3D11DeviceContext& context3D, PixelShaderView ps)
    {
        context3D.PSSetShader(ps.Shader, nullptr, 0);
        const auto cbs = ps.Cbs;
        context3D.PSSetConstantBuffers(0, static_cast<UINT>(cbs.size()), cbs.data());
    }

    VertexShader VertexShader::CompileFromFile(ID3D11Device& device,
        const fs::path& filePath,
        const char* entryName,
        gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc)
    {
        auto byteCode = dx::CompileShaderFromFile(filePath.c_str(), entryName, "vs_5_0");
        return FromByteCode(device, BlobToSpan(Ref(byteCode)), layoutDesc);
    }

    VertexShader VertexShader::FromByteCode(ID3D11Device& device, gsl::span<const std::byte> byteCode, gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc)
    {
        VertexShader vs;
        TryHR(device.CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, vs.shader_.ReleaseAndGetAddressOf()));
        TryHR(device.CreateInputLayout(layoutDesc.data(), layoutDesc.size(), byteCode.data(),
            byteCode.size(), vs.layout_.GetAddressOf()));
        return vs;
    }

    std::optional<VertexShader> VertexShader::Find(std::uint32_t tag)
    {
        const auto& vertexShaders = GetGame().vertexShaders_;
        const auto it = vertexShaders.find(tag);
        if (it != vertexShaders.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void VertexShader::AddShader(std::uint32_t tag, VertexShader vs)
    {
        auto& vertexShaders = GetGame().vertexShaders_;
        vertexShaders.insert({ tag, std::move(vs) });
    }

    std::optional<PixelShader> PixelShader::Find(std::uint32_t tag)
    {
        const auto& pixelShaders = GetGame().pixelShaders_;
        const auto it = pixelShaders.find(tag);
        if (it != pixelShaders.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void PixelShader::AddShader(std::uint32_t tag, PixelShader ps)
    {
        auto& pixelShaders = GetGame().pixelShaders_;
        pixelShaders.insert({ tag, std::move(ps) });
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
        return { shader_.Get(), layout_.Get(), ComPtrsCast(gsl::make_span(Cbs))};
    }

    VertexShader::~VertexShader()
    {
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

    PixelShader PixelShader::FromByteCode(ID3D11Device& device, gsl::span<const std::byte> byteCode)
    {
        return { CreatePixelShader(device, byteCode) };
    }

    ID3D11PixelShader& PixelShader::GetShader() const noexcept
    {
        return Ref(shader_);
    }

    PixelShaderView PixelShader::Get() const noexcept
    {
        return {
            shader_.Get(),
            ComPtrsCast(gsl::make_span(Cbs))
        };
    }

    PixelShader::~PixelShader()
    {
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
