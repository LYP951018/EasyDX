#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct IDXGISwapChain;
struct ID3D11Buffer;
struct ID3D10Blob;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11InputLayout;
struct IDXGIDevice;
struct IDXGIDevice1;
struct ID3D11Debug;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11Resource;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11DeviceChild;
struct ID3D11ShaderReflectionConstantBuffer;
struct ID3D11ShaderReflection;

struct ID2D1Factory1;
struct ID2D1Device;
struct ID2D1DeviceContext;
struct ID2D1Bitmap;
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
struct ID2D1Bitmap1;

struct IDWriteFactory1;
struct IDWriteTextLayout1;
struct IDWriteTextFormat;

struct IDXGISwapChain;
struct IDXGIFactory;

struct D3D11_INPUT_ELEMENT_DESC;
struct D3D11_VIEWPORT;

namespace dx
{
    enum class ResourceUsage
    {
        Default, // requires read and write access by the GPU
        Immutable, // can only be read by the GPU
        Dynamic, // accessible by both the GPU (read only) and the CPU (write only).
        Staging // supports data transfer (copy) from the GPU to the CPU.
    };

    enum class BindFlag
    {
        VertexBuffer = 0x1,
        IndexBuffer = 0x2,
        ConstantBuffer = 0x4,
        ShaderResource = 0x8,
        StreamOutput = 0x10,
        RenderTarget = 0x20,
        DepthStencil = 0x40,
        UnorderedAccess = 0x80,
        Decoder = 0x200,
        VideoEncoder = 0x400
    };

    //TODO: Fill them all.
    enum class DxgiFormat
    {
        //A four-component, 128-bit floating-point format that supports 32 bits per channel including alpha.
        R32G32B32A32Float = 2,
        //A three-component, 96-bit floating-point format that supports 32 bits per color channel.
        R32G32B32Float = 6,
        R32G32Float = 16,
        //A single-component, 16-bit unsigned-integer format that supports 16 bits for the red channel.
        R16UInt = 57,
        B8G8R8A8UNorm = 87,
        //A 32-bit z-buffer format that supports 24 bits for depth and 8 bits for stencil.
        Depth24UNormStencilS8UInt = 45
    };

    enum class ResourceMapType
    {
        Read = 1,
        Write = 2,
        ReadWrite = 3,
        WriteDiscard = 4,
        WriteNoOverwrite = 5
    };

    template<DxgiFormat>
    struct dxgi_format_map;

    template<>
    struct dxgi_format_map<DxgiFormat::R16UInt>
    {
        using type = std::uint16_t;
    };

    inline constexpr std::uint32_t kMaxStreamCount = 8;

    template<typename T>
    using MaxStreamVector = boost::container::static_vector<T, kMaxStreamCount>;
}

