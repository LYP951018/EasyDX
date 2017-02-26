#pragma once

#include "Common.hpp"
#include <cstdint>
#include <string>
#include <wrl/client.h>
#include <gsl/span>

namespace dx
{
    struct SwapChainOptions
    {
        std::uint32_t backBufferCount = 2;
        //TODO: remove the ugly hack
        /*DXGI_FORMAT*/int backBufferFormat = 28/*DXGI_FORMAT_B8G8R8A8_UNORM*/,
            depthBufferFormat = 45/*DXGI_FORMAT_D24_UNORM_S8_UINT*/;
        bool Windowed = true;
    };

    struct ViewportOptions
    {
        float TopLeftX = 0.f, TopLeftY = 0.f;
        float Width = {}, Height = {};
        float MinDepth = 0.f, MaxDepth = 1.f;
    };

    class GameWindow
    {
    public:
        GameWindow(const std::wstring& title = L"DirectX",
            std::uint32_t width = UINT32_MAX,
            std::uint32_t height = UINT32_MAX,
            SwapChainOptions options = {});

        GameWindow(const GameWindow&) = delete;
        GameWindow& operator= (const GameWindow&) = delete;

        struct Win32Params;

       
        //public ?
        std::int32_t ProcessMessage(std::uint32_t message, Win32Params params);

        void Show();
        void Relocate(const Rect& rect);
        void* GetNativeHandle() const noexcept;
        IDXGISwapChain* GetSwapChain() const;
        ID3D11RenderTargetView* GetBackBuffer() const;
        ID3D11DepthStencilView* GetDepthBuffer() const;
        std::uint32_t GetWidth() const noexcept;
        std::uint32_t GetHeight() const noexcept;
        float GetDpiX() const noexcept;
        float GetDpiY() const noexcept;

        virtual ~GameWindow();

    protected:
        void Present();

        virtual void Render(ID3D11DeviceContext& context3D, ID2D1DeviceContext& context2D);
        virtual void OnResize(std::uint32_t newWidth, std::uint32_t newHeight);
        virtual void OnDpiUpdated(std::uint32_t dpiX, std::uint32_t dpiY);

        void ResetD3D();
        void Clear(gsl::span<float, 4> color,
            const ViewportOptions& viewPortOptions);
        void ClearWithDefault();

    private:
        void OnPaint();
        void PrepareForResize(std::uint32_t newWidth, std::uint32_t newHeight);
        void UpdateDpi(std::uint32_t dpiX, std::uint32_t dpiY, const Rect& newWindowRect);
        void CreateResources();
        void CreateSwapChain();
        void InitializeDpi();

        void* windowHandle_;
        std::uint32_t width_, height_;
        std::uint32_t dpiX_, dpiY_;

        SwapChainOptions swapChainOptions_;
        wrl::ComPtr<IDXGISwapChain> swapChain_;
        wrl::ComPtr<ID3D11RenderTargetView> backBufferRenderTargetView_;
        wrl::ComPtr<ID3D11DepthStencilView> depthBufferRenderTargetView_;

        wrl::ComPtr<ID2D1Bitmap1> targetBitmap_;
    };
}