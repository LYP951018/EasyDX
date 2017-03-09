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
struct ID3D11InputLayout;
struct IDXGIDevice;
struct IDXGIDevice1;

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

struct D3D11_INPUT_ELEMENT_DESC;

namespace dx
{
    enum class BufferUsage
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
}

