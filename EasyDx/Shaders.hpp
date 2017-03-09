#pragma once

#include "Common.hpp"
#include "FileSystemAlias.hpp"
#include <gsl/span>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, ID3D10Blob& blob);
    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, ID3D10Blob& blob);

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

    //对 InputLayout 与 VertexShader 的浅封装。无需使用 shared_ptr，直接 copy，因为其类成员都使用 wrl::ComPtr。
    class VertexShader
    {
    public:
        VertexShader() = default;

        static VertexShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName,
            gsl::span<D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        void Bind(ID3D11DeviceContext& deviceContext);

    private:
        VertexShader(
            wrl::ComPtr<ID3D11VertexShader> shader,
            wrl::ComPtr<ID3D11InputLayout> layout);

        wrl::ComPtr<ID3D11VertexShader> shader_;
        wrl::ComPtr<ID3D11InputLayout> layout_;
    };

    struct PixelShader final
    {
    public:
        PixelShader() = delete;

        static wrl::ComPtr<ID3D11PixelShader> CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName);
    };
}