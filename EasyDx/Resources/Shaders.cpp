#include "../pch.hpp"
#include "Shaders.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
#define CREATE_SHADER_DEFINE(shaderName) \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(::ID3D11Device& device, gsl::span<const std::byte> byteCode)\
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

   
}
