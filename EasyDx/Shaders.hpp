#pragma once

#include "FileSystemAlias.hpp"
#include "SimpleVertex.hpp"
#include <optional>

namespace dx
{
    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, gsl::span<const std::byte> byteCode);
    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, gsl::span<const std::byte> byteCode);

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

    struct VertexShaderView
    {
        ID3D11VertexShader* Shader;
        ID3D11InputLayout* Layout;
        gsl::span<const Ptr<ID3D11Buffer>> Cbs;
    };

    struct VertexShaderKind
    {
        enum : std::uint32_t
        {
            kBasic = 0
        };

        VertexShaderKind() = delete;
    };

    struct VertexShader
    {
    public:
        VertexShader() = default;

        static VertexShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        static VertexShader FromByteCode(ID3D11Device& device,
            gsl::span<const std::byte> byteCode,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        template<std::size_t N>
        static VertexShader FromByteCode(ID3D11Device& device,
            const unsigned char(&byteCode)[N])
        {
            const auto p = byteCode;
            return FromByteCode(device, gsl::make_span(reinterpret_cast<const std::byte*>(p),
                static_cast<std::ptrdiff_t>(N)), SimpleVertex::GetLayout());
        }

        static std::optional<VertexShader> Find(std::uint32_t tag);
        static void AddShader(std::uint32_t tag, VertexShader vs);

        ID3D11VertexShader& GetShader() const;
        ID3D11InputLayout& GetLayout() const;

        VertexShaderView Get() const noexcept;

        std::vector<wrl::ComPtr<ID3D11Buffer>> Cbs;
        ~VertexShader();

    private:
        VertexShader(wrl::ComPtr<ID3D11VertexShader> shader,
            wrl::ComPtr<ID3D11InputLayout> layout);

        wrl::ComPtr<ID3D11VertexShader> shader_;
        wrl::ComPtr<ID3D11InputLayout> layout_;
    };

    struct PixelShaderView
    {
        ID3D11PixelShader* Shader;
        gsl::span<const Ptr<ID3D11Buffer>> Cbs;
    };

    struct PixelShader
    {
    public:
        PixelShader() = default;

        static PixelShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName);

        static PixelShader FromByteCode(ID3D11Device& device,
            gsl::span<const std::byte> byteCode);

        template<std::size_t N>
        static PixelShader FromByteCode(ID3D11Device& device,
            const unsigned char(&byteCode)[N])
        {
            const auto p = byteCode;
            return FromByteCode(device, gsl::make_span(reinterpret_cast<const std::byte*>(p),
                static_cast<std::ptrdiff_t>(N)));
        }

        static std::optional<PixelShader> Find(std::uint32_t tag);
        static void AddShader(std::uint32_t tag, PixelShader ps);

        ID3D11PixelShader& GetShader() const noexcept;
        PixelShaderView Get() const noexcept;

        ~PixelShader();

        std::vector<wrl::ComPtr<ID3D11Buffer>> Cbs;

    private:
        PixelShader(wrl::ComPtr<ID3D11PixelShader> shader);

        wrl::ComPtr<ID3D11PixelShader> shader_;
    };

    void SetupShader(ID3D11DeviceContext& context3D, VertexShaderView vs);
    void SetupShader(ID3D11DeviceContext& context3D, PixelShaderView ps);

    struct PixelShaderKind
    {
        enum : std::uint32_t
        {
            kBasic = 0
        };

        PixelShaderKind() = delete;
    };
}