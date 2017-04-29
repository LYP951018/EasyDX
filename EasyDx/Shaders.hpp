#pragma once

#include "Common.hpp"
#include "FileSystemAlias.hpp"
#include <gsl/span>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, gsl::span<const gsl::byte> byteCode);
    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, gsl::span<const gsl::byte> byteCode);

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

    struct VertexShaderView
    {
        ID3D11VertexShader* Shader;
        ID3D11InputLayout* Layout;
        gsl::span<const Ptr<ID3D11Buffer>> ConstantBuffers;
    };

    class VertexShader
    {
    public:
        VertexShader() = default;

        static VertexShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        static VertexShader FromByteCode(ID3D11Device& device,
            gsl::span<const gsl::byte> byteCode,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        void Bind(ID3D11DeviceContext& deviceContext) const;

        ID3D11VertexShader& GetShader() const;
        ID3D11InputLayout& GetLayout() const;

        VertexShaderView Get() const noexcept;

        std::vector<wrl::ComPtr<ID3D11Buffer>> ConstantBuffers;

    private:
        VertexShader(
            wrl::ComPtr<ID3D11VertexShader> shader,
            wrl::ComPtr<ID3D11InputLayout> layout);

        wrl::ComPtr<ID3D11VertexShader> shader_;
        wrl::ComPtr<ID3D11InputLayout> layout_;
    };

    struct PixelShaderView
    {
        ID3D11PixelShader* Shader;
        gsl::span<const Ptr<ID3D11Buffer>> ConstantBuffers;
    };

    struct PixelShader
    {
    public:
        PixelShader() = default;

        static PixelShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName);

        static PixelShader FromByteCode(ID3D11Device& device,
            gsl::span<const gsl::byte> byteCode);

        void Bind(ID3D11DeviceContext& deviceContext) const;

        ID3D11PixelShader& GetShader() const noexcept;

        PixelShaderView Get() const noexcept;

        std::vector<wrl::ComPtr<ID3D11Buffer>> ConstantBuffers;

    private:
        PixelShader(wrl::ComPtr<ID3D11PixelShader> shader);

        wrl::ComPtr<ID3D11PixelShader> shader_;
    };
}