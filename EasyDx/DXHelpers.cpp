#include "DXHelpers.hpp"
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace dx
{
    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(
            ID3D11Device& device,
            const void* buffer,
            std::size_t bufferSize,
            std::uint32_t bindFlags)
        {
            wrl::ComPtr<ID3D11Buffer> d3dBuffer;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = static_cast<UINT>(bufferSize);
            bufferDesc.BindFlags = static_cast<UINT>(bindFlags);
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = buffer;
            dx::TryHR(device.CreateBuffer(&bufferDesc, &initData, d3dBuffer.GetAddressOf()));
            return d3dBuffer;
        }
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
}


