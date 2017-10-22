#include "pch.hpp"
#include "Shaders.hpp"
#include "Game.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
#define CREATE_SHADER_DEFINE(shaderName) \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(ID3D11Device& device, gsl::span<const std::byte> byteCode)\
    {\
        wrl::ComPtr<ID3D11##shaderName> shader;\
        TryHR(device.Create##shaderName(byteCode.data(), byteCode.size(), nullptr, shader.GetAddressOf()));\
        return shader;\
    }\

    CREATE_SHADER_DEFINE(VertexShader)
    CREATE_SHADER_DEFINE(PixelShader)
    CREATE_SHADER_DEFINE(HullShader)
    CREATE_SHADER_DEFINE(DomainShader)

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
        TryHR(device.CreateInputLayout(layoutDesc.data(), static_cast<UINT>(layoutDesc.size()), byteCode.data(),
            byteCode.size(), vs.layout_.GetAddressOf()));
        return vs;
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

    VertexShader::~VertexShader()
    {
    }

    VertexShader::VertexShader(wrl::ComPtr<ID3D11VertexShader> shader, wrl::ComPtr<ID3D11InputLayout> layout)
        : shader_{std::move(shader)},
        layout_{std::move(layout)}
    {
    }

#define DEFINE_SHADER_Def(shaderName, shaderModel)\
    shaderName shaderName::CompileFromFile(ID3D11Device& device, const fs::path& filePath, const char* entryName)\
    {\
        auto byteCode = CompileShaderFromFile(\
            filePath.c_str(),\
            entryName,\
            #shaderModel\
        );\
        return FromByteCode(device, BlobToSpan(Ref(byteCode)));\
    }\
    shaderName shaderName::FromByteCode(ID3D11Device& device, gsl::span<const std::byte> byteCode)\
    {\
        return { Create##shaderName(device, byteCode) };\
    }\
    ID3D11##shaderName& shaderName::GetShader() const noexcept\
    {\
        return Ref(shader_);\
    }\
    shaderName##View shaderName::Get() const noexcept\
    {\
        return shader_.Get();\
    }\
    shaderName::~shaderName(){}\
    shaderName::shaderName(wrl::ComPtr<ID3D11##shaderName> shader)\
        : shader_{std::move(shader)} {}

    DEFINE_SHADER_Def(PixelShader, ps_5_0)
    DEFINE_SHADER_Def(HullShader, hs_5_0)
    DEFINE_SHADER_Def(DomainShader, ds_5_0)


    void SetShaders(ID3D11DeviceContext& context, const ShaderViewGroup& shaders)
    {
        auto vs = shaders.VS;
        context.IASetInputLayout(vs.Layout);
        context.VSSetShader(vs.Shader, nullptr, 0);
        context.PSSetShader(shaders.PS, nullptr, 0);
        context.HSSetShader(shaders.HS, nullptr, 0);
        context.DSSetShader(shaders.DS, nullptr, 0);
    }
}
