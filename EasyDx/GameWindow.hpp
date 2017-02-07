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
            bool fullScreen = false,
            std::uint32_t width = UINT32_MAX,
            std::uint32_t height = UINT32_MAX,
            SwapChainOptions options = {});

        GameWindow(const GameWindow&) = delete;
        GameWindow& operator= (const GameWindow&) = delete;

        struct Win32Params;

       
        //public ?
        std::int32_t ProcessMessage(std::uint32_t message, Win32Params params);

        void Show();
        void* GetNativeHandle() const noexcept;
        IDXGISwapChain* GetSwapChain() const;
        ID3D11RenderTargetView* GetBackBuffer() const;
        ID3D11DepthStencilView* GetDepthBuffer() const;
        std::uint32_t GetWidth() const noexcept;
        std::uint32_t GetHeight() const noexcept;

        virtual ~GameWindow();

    protected:
        void Present();
        virtual void Render(ID3D11DeviceContext& context);
        virtual void OnResize(std::uint32_t newWidth, std::uint32_t newHeight);
        void ResetD3D();
        void Clear(gsl::span<float, 4> color,
            const ViewportOptions& viewPortOptions);
        void ClearWithDefault();

    private:
        void OnPaint();
        void OnResizeInternal(std::uint32_t newWidth, std::uint32_t newHeight);
        void CreateResources();
        void CreateSwapChain();

        void* windowHandle_;
        std::uint32_t width_, height_;
        SwapChainOptions swapChainOptions_;
        wrl::ComPtr<IDXGISwapChain> swapChain_;
        wrl::ComPtr<ID3D11RenderTargetView> backBufferRenderTargetView_;
        wrl::ComPtr<ID3D11DepthStencilView> depthBufferRenderTargetView_;
    };
}