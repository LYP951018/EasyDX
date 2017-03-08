﻿#pragma once

#include <string>
#include <wrl/client.h>
//#include <type_traits>

namespace wrl = Microsoft::WRL;

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

struct tagRECT;

namespace dx
{
    std::string ws2s(const std::wstring& wstr);

    [[noreturn]]
    void ThrowHRException(long hr);

    void TryHR(long hr);

    void TryWin32(int b);

    struct Rect
    {
        std::uint32_t LeftTopX, LeftTopY, Width, Height;
        static Rect FromRECT(const tagRECT& win32Rect) noexcept;
    };
}
