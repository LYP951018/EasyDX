#include "GameWindow.hpp"
#include "Game.hpp"
#include "Common.hpp"
#include <gsl/gsl_assert>
#include <Windows.h>
#include <d3d11.h>

namespace dx
{
    ::HINSTANCE GetInstanceHandle()
    {
        const static ::HINSTANCE instance = ::GetModuleHandle(nullptr);
        Ensures(instance != nullptr);
        return instance;
    }

    struct GameWindow::Win32Params
    {
        std::uintptr_t wParam, lParam;
    };

    ::HWND GetWin32WindowHandle(const GameWindow& window) noexcept
    {
        return static_cast<::HWND>(window.GetNativeHandle());
    }

    ::LRESULT CALLBACK WindowProc(::HWND windowHandle, ::UINT messageId, ::WPARAM wParam, ::LPARAM lParam)
    {
        GameWindow* window = {};
        if (messageId == WM_NCCREATE)
        {
            const auto pcs = reinterpret_cast<::CREATESTRUCT*>(lParam);
            window = static_cast<GameWindow*>(pcs->lpCreateParams);
            Ensures(window != nullptr);
            ::SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<::LONG_PTR>(window));
        }
        else
        {
            window = reinterpret_cast<GameWindow*>(::GetWindowLongPtr(windowHandle, GWLP_USERDATA));
            if (window != nullptr)
            {
                window->ProcessMessage(static_cast<std::uint32_t>(messageId), { wParam, static_cast<std::uintptr_t>(lParam) });
            }
        }
        return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
    }

    GameWindow::GameWindow(const std::wstring& title, bool fullScreen, std::uint32_t width, std::uint32_t height,
        SwapChainOptions options)
        : swapChainOptions_{options}
    {
        ::WNDCLASSEX windowClass = {};
        windowClass.cbSize = sizeof(::WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = GetInstanceHandle();
        windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = L"DXWindow";
        windowClass.cbWndExtra = sizeof(std::intptr_t);
        ::RegisterClassEx(&windowClass);
        windowHandle_ = ::CreateWindow(L"DXWindow", title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
            width == UINT32_MAX ? CW_USEDEFAULT : width,
            height == UINT32_MAX ? CW_USEDEFAULT : height,
            nullptr, nullptr, GetInstanceHandle(), static_cast<void*>(this));
        Ensures(windowHandle_ != nullptr);
        //CreateResources();
        CreateSwapChain();
    }

    void GameWindow::Show()
    {
        ::ShowWindow(GetWin32WindowHandle(*this), SW_SHOW);
        ::UpdateWindow(GetWin32WindowHandle(*this));
    }

    void* GameWindow::GetNativeHandle() const noexcept
    {
        return windowHandle_;
    }

    IDXGISwapChain* GameWindow::GetSwapChain() const
    {
        return swapChain_.Get();
    }

    ID3D11RenderTargetView* GameWindow::GetBackBuffer() const
    {
        return backBufferRenderTargetView_.Get();
    }

    ID3D11DepthStencilView* GameWindow::GetDepthBuffer() const
    {
        return depthBufferRenderTargetView_.Get();
    }

    std::uint32_t GameWindow::GetWidth() const noexcept
    {
        return width_;
    }

    std::uint32_t GameWindow::GetHeight() const noexcept
    {
        return height_;
    }

    GameWindow::~GameWindow()
    {
    }

    void GameWindow::Render(ID3D11DeviceContext& context)
    {
    }

    void GameWindow::OnResize(std::uint32_t newWidth, std::uint32_t newHeight)
    {
    }

    std::int32_t GameWindow::ProcessMessage(std::uint32_t message, Win32Params params)
    {
        switch (message)
        {
        case WM_PAINT:
            OnPaint();
        break;
        case WM_SIZE:
        {
            const auto lParam = params.lParam;
            const auto newWidth = static_cast<std::uint32_t>(lParam & 0xFFFF);
            const auto newHeight = static_cast<std::uint32_t>(lParam >> 16);
            OnResizeInternal(newWidth, newHeight);
        }
        break;
        case WM_QUIT:
            ::PostQuitMessage(0);
            break;
        default:
            break;
        }
        return 0;
    }

    void GameWindow::OnPaint()
    {
        Render(GetDeviceContext());
        Present();
    }

    void GameWindow::OnResizeInternal(std::uint32_t newWidth, std::uint32_t newHeight)
    {
        width_ = newWidth;
        height_ = newHeight;
        ResetD3D();
        TryHR(swapChain_-> ResizeBuffers(2, newWidth, newHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
        CreateResources();
        OnResize(newWidth, newHeight);
    }

    void GameWindow::Present()
    {
        TryHR(swapChain_->Present(0, 0));
    }

    void GameWindow::CreateResources()
    {
        auto& device = GetD3DDevice();
        wrl::ComPtr<ID3D11Texture2D> backBuffer;
        TryHR(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf())));
        device.CreateRenderTargetView(backBuffer.Get(), {}, backBufferRenderTargetView_.ReleaseAndGetAddressOf());
        CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, width_, height_, 1, 1, D3D11_BIND_DEPTH_STENCIL);

        wrl::ComPtr<ID3D11Texture2D> depthStencil;
        TryHR(device.CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));
        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
        TryHR(device.CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, depthBufferRenderTargetView_.ReleaseAndGetAddressOf()));
    }

    void GameWindow::ResetD3D()
    {
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        auto& deviceContext = GetDeviceContext();
        deviceContext.OMSetRenderTargets(std::size(nullViews), nullViews, nullptr);
        backBufferRenderTargetView_.Reset();
        depthBufferRenderTargetView_.Reset();
        deviceContext.Flush();
    }

    void GameWindow::Clear(gsl::span<float, 4> color, const ViewportOptions& viewportOptions)
    {
        auto& deviceContext = GetDeviceContext();
        deviceContext.ClearRenderTargetView(backBufferRenderTargetView_.Get(), color.data());
        deviceContext.ClearDepthStencilView(depthBufferRenderTargetView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
        ID3D11RenderTargetView* views[] = { backBufferRenderTargetView_.Get() };
        deviceContext.OMSetRenderTargets(std::size(views), views, depthBufferRenderTargetView_.Get());
        const D3D11_VIEWPORT viewport{
            viewportOptions.TopLeftX,
            viewportOptions.TopLeftY,
            viewportOptions.Width,
            viewportOptions.Height,
            viewportOptions.MinDepth,
            viewportOptions.MaxDepth
        };
        deviceContext.RSSetViewports(1, &viewport);
    }

    void GameWindow::ClearWithDefault()
    {
        float color[] = { 1.f, 1.f, 1.f, 1.f };
        ViewportOptions options = {
            0.f, 0.f, static_cast<float>(width_), static_cast<float>(height_), 0.f, 1.f
        };
        Clear({ color }, options);
    }

    void GameWindow::CreateSwapChain()
    {
        wrl::ComPtr<IDXGIDevice> dxgiDevice;
        auto& d3dDevice = GetD3DDevice();
        TryHR(d3dDevice.QueryInterface(dxgiDevice.ReleaseAndGetAddressOf()));
        wrl::ComPtr<IDXGIAdapter> adapter;
        TryHR(dxgiDevice->GetAdapter(adapter.ReleaseAndGetAddressOf()));
        wrl::ComPtr<IDXGIFactory> dxgiFactory;
        TryHR(adapter->GetParent(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = swapChainOptions_.backBufferCount;
        desc.BufferDesc.Width = 0;
        desc.BufferDesc.Height = 0;
        desc.BufferDesc.Format = static_cast<DXGI_FORMAT>(swapChainOptions_.backBufferFormat);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = GetWin32WindowHandle(*this);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Windowed = swapChainOptions_.Windowed == true ? 1 : 0;
        TryHR(dxgiFactory->CreateSwapChain(&d3dDevice, &desc, swapChain_.ReleaseAndGetAddressOf()));
    }
}
