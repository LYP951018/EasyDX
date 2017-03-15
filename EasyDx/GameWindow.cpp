#include "GameWindow.hpp"
#include "Game.hpp"
#include "Common.hpp"
#include "Scene.hpp"
#include "Misc.hpp"
#include <gsl/gsl_assert>
#include <Windows.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <windowsx.h>

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

    GameWindow::GameWindow(const std::wstring& title, std::uint32_t width, std::uint32_t height,
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
        InitializeDpi();
        CreateSwapChain();
    }

    void GameWindow::Show()
    {
        ::ShowWindow(GetWin32WindowHandle(*this), SW_SHOW);
        ::UpdateWindow(GetWin32WindowHandle(*this));
    }

    void GameWindow::Relocate(const Rect& rect)
    {
        TryWin32(SetWindowPos(GetWin32WindowHandle(*this),
            HWND_TOP,
            rect.LeftTopX,
            rect.LeftTopY,
            rect.Width,
            rect.Height,
            SWP_NOZORDER | SWP_NOACTIVATE));
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

    float GameWindow::GetDpiX() const noexcept
    {
        return static_cast<float>(dpiX_);
    }

    float GameWindow::GetDpiY() const noexcept
    {
        return static_cast<float>(dpiY_);
    }

    GameWindow::~GameWindow()
    {
    }

    void GameWindow::Render(ID3D11DeviceContext& context3D, ID2D1DeviceContext& context2D)
    {
        auto mainScene = GetGame().GetMainScene();
        mainScene->Render(context3D, context2D);
    }

    Point PosFromLParam(LPARAM lParam) noexcept
    {
        return { static_cast<std::int32_t>(GET_X_LPARAM(lParam)), 
            static_cast<std::int32_t>(GET_Y_LPARAM(lParam)) };
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
            PrepareForResize(newWidth, newHeight);
            ResizeEventArgs args;
            args.Window = this;
            args.NewSize = { newWidth, newHeight };
            WindowResize(args);
        }
        break;
        case WM_QUIT:
            ::PostQuitMessage(0);
            break;
        case WM_DPICHANGED:
        {
            const auto wParam = params.wParam;
            const auto newDpiX = static_cast<std::uint32_t>(LOWORD(wParam));
            const auto newDpiY = static_cast<std::uint32_t>(HIWORD(wParam));
            const auto rect = *reinterpret_cast<RECT*>(params.lParam);
            const auto newWindowRect = Rect::FromRECT(rect);
            UpdateDpi(newDpiX, newDpiY, newWindowRect);
            DpiChangedEventArgs args;
            args.Window = this;
            args.NewDpiX = newDpiX;
            args.NewDpiY = newDpiY;
            DpiChanged(args);
        }
        break;
        case WM_KEYDOWN:
        {
            KeyEventArgs args;
            args.Window = this;
            args.Key = static_cast<std::uint32_t>(params.wParam);
            KeyDown(args);
        }
        break;
        case WM_KEYUP:
        {
            KeyEventArgs args;
            args.Window = this;
            args.Key = static_cast<std::uint32_t>(params.wParam);
            KeyUp(args);
        }
        break;
        case WM_LBUTTONDOWN:
        {
            MouseEventArgs args;
            args.Window = this;
            args.Button = MouseButton::kLeft;
            args.Position = PosFromLParam(params.lParam);
            MouseDown(args);
        }
        break;
        case WM_LBUTTONUP:
        {
            MouseEventArgs args;
            args.Window = this;
            args.Button = MouseButton::kLeft;
            args.Position = PosFromLParam(params.lParam);
            MouseUp(args);
        }
        break;
        case WM_RBUTTONDOWN:
        {
            MouseEventArgs args;
            args.Window = this;
            args.Button = MouseButton::kRight;
            args.Position = PosFromLParam(params.lParam);
            MouseDown(args);
        }
        break;
        case WM_RBUTTONUP:
        {
            MouseEventArgs args;
            args.Window = this;
            args.Button = MouseButton::kRight;
            args.Position = PosFromLParam(params.lParam);
            MouseUp(args);
        }
        break;
        default:
            break;
        }
        return 0;
    }

    void GameWindow::OnPaint()
    {
        auto& game = GetGame();
        Render(game.GetContext3D(), game.GetContext2D());
        Present();
    }

    void GameWindow::PrepareForResize(std::uint32_t newWidth, std::uint32_t newHeight)
    {
        width_ = newWidth;
        height_ = newHeight;
        ResetD3D();
        TryHR(swapChain_-> ResizeBuffers(2, newWidth, newHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
        CreateResources();
    }

    void GameWindow::UpdateDpi(std::uint32_t dpiX, std::uint32_t dpiY, const Rect& newWindowRect)
    {
        dpiX_ = dpiX;
        dpiY_ = dpiY;
        Relocate(newWindowRect);
    }

    void GameWindow::Present()
    {
        TryHR(swapChain_->Present(0, 0));
    }

    void GameWindow::CreateResources()
    {
        auto& game = GetGame();
        auto& device = game.GetDevice3D();
        wrl::ComPtr<ID3D11Texture2D> backBuffer;
        TryHR(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf())));
        device.CreateRenderTargetView(backBuffer.Get(), {}, backBufferRenderTargetView_.ReleaseAndGetAddressOf());
        CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, width_, height_, 1, 1, D3D11_BIND_DEPTH_STENCIL);

        wrl::ComPtr<ID3D11Texture2D> depthStencil;
        TryHR(device.CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));
        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
        TryHR(device.CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, depthBufferRenderTargetView_.ReleaseAndGetAddressOf()));
    
        const auto props = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            static_cast<float>(GetDpiX()),
            static_cast<float>(GetDpiY())
        );
       wrl::ComPtr<IDXGISurface> surface;
       TryHR(swapChain_->GetBuffer(0, IID_PPV_ARGS(surface.ReleaseAndGetAddressOf())));
       auto& context2D = game.GetContext2D();
       context2D.CreateBitmapFromDxgiSurface(surface.Get(), &props, targetBitmap_.ReleaseAndGetAddressOf());
       context2D.SetTarget(targetBitmap_.Get());
    }

    void GameWindow::ResetD3D()
    {
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        auto& game = GetGame();
        auto& context3D = game.GetContext3D();
        auto& context2D = game.GetContext2D();
        context3D.OMSetRenderTargets(std::size(nullViews), nullViews, nullptr);
        context2D.SetTarget(nullptr);
        targetBitmap_.Reset();
        backBufferRenderTargetView_.Reset();
        depthBufferRenderTargetView_.Reset();
        context3D.Flush();
    }

    void GameWindow::Clear(DirectX::XMVECTOR color)
    {
        auto& game = GetGame();
        auto& deviceContext = game.GetContext3D();
        DirectX::XMFLOAT4 colorFloats;
        DirectX::XMStoreFloat4(&colorFloats, color);
        deviceContext.ClearRenderTargetView(backBufferRenderTargetView_.Get(), reinterpret_cast<const float*>(&colorFloats));
        deviceContext.ClearDepthStencilView(depthBufferRenderTargetView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
        ID3D11RenderTargetView* views[] = { backBufferRenderTargetView_.Get() };
        //FIXME: is this necessary?
        deviceContext.OMSetRenderTargets(std::size(views), views, depthBufferRenderTargetView_.Get());
        
        const auto& mainViewport = game.GetMainScene()->GetMainCamera().MainViewport;
        const D3D11_VIEWPORT viewport{
           mainViewport.TopLeftX,
           mainViewport.TopLeftY,
           mainViewport.Width,
           mainViewport.Height,
           mainViewport.MinDepth,
           mainViewport.MaxDepth
        };
        deviceContext.RSSetViewports(1, &viewport);
    }

    void GameWindow::CreateSwapChain()
    {
        auto& game = GetGame();
        auto& dxgiDevice = game.GetDxgiDevice();
        auto& d3dDevice = game.GetDevice3D();
        wrl::ComPtr<IDXGIAdapter> adapter;
        TryHR(dxgiDevice.GetAdapter(adapter.ReleaseAndGetAddressOf()));
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

    void GameWindow::InitializeDpi()
    {
        const auto dpi = GetDpiForWindow(GetWin32WindowHandle(*this));
        Ensures(dpi != 0);
        dpiX_ = static_cast<std::uint32_t>(dpi);
        dpiY_ = static_cast<std::uint32_t>(dpi);
    }
}
