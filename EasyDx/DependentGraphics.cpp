#include "pch.hpp"
#include "DependentGraphics.hpp"
#include "GameWindow.hpp"
#include "D3DHelpers.hpp"
#include <d3d11.h>

using namespace std::literals;

namespace dx
{
    namespace Internal
    {
        template<typename T>
        wrl::ComPtr<T> GetParent(IDXGIObject& object)
        {
            void* ptr;
            TryHR(object.GetParent(__uuidof(T), &ptr));
            return { static_cast<T*>(ptr) };
        }
    }

    SwapChain::SwapChain(ID3D11Device& device, const GameWindow& window, const SwapChainOptions& options)
        : options_{ options }
    {
        wrl::ComPtr<IDXGIDevice1> dxgiDevice;
        TryHR(device.QueryInterface(dxgiDevice.GetAddressOf()));
        const auto adapter = Internal::GetParent<IDXGIAdapter1>(Ref(dxgiDevice));
        const auto factory = Internal::GetParent<IDXGIFactory1>(Ref(adapter));
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = gsl::narrow<UINT>(options.CountOfBackBuffers);
        desc.BufferDesc.Width = gsl::narrow<UINT>(window.GetWidth());
        desc.BufferDesc.Height = gsl::narrow<UINT>(window.GetHeight());
        desc.BufferDesc.Format = static_cast<DXGI_FORMAT>(options.BackBufferFormat);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = window.NativeHandle();
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Windowed = options.Windowed == true ? TRUE : FALSE;
        TryHR(factory->CreateSwapChain(&device, &desc, swapChain_.GetAddressOf()));
        UpdateBuffers(device);
    }

    SwapChain::~SwapChain()
    {
    }

    auto SwapChain::Front() -> BackBuffer&
    {
        return backBuffer_;
    }

    void SwapChain::Present()
    {
        TryHR(swapChain_->Present(0, 0));
    }

    void SwapChain::Reset()
    {
        backBuffer_.Reset();
    }

    //https://stackoverflow.com/questions/28095798/how-to-change-window-size-in-directx-11-desktop-application
    void SwapChain::Resize(ID3D11Device& device, std::uint32_t height, std::uint32_t width)
    {
        TryHR(swapChain_->ResizeBuffers(options_.CountOfBackBuffers, gsl::narrow<UINT>(height), gsl::narrow<UINT>(width), static_cast<DXGI_FORMAT>(options_.BackBufferFormat), 0));
        UpdateBuffers(device);
    }

    void SwapChain::UpdateBuffers(ID3D11Device& device)
    {
        //https://msdn.microsoft.com/en-us/library/windows/desktop/mt427784%28v=vs.85%29.aspx
        //In Direct3D 11, applications could call GetBuffer(0, бн) only once.Every call to Present implicitly changed the resource identity of the returned interface.
        backBuffer_ = BackBuffer{device, GetBuffer(Ref(swapChain_), 0) };
    }

    wrl::ComPtr<ID3D11Texture2D> GetBuffer(IDXGISwapChain& swapChain, std::uint32_t index)
    {
        wrl::ComPtr<ID3D11Texture2D> tex;
        void* ptr;
        TryHR(swapChain.GetBuffer(gsl::narrow<UINT>(index), __uuidof(ID3D11Texture2D), &ptr));
        tex.Attach(static_cast<ID3D11Texture2D*>(ptr));
        //leak: return wrl::ComPtr<ID3D11Texture2D>{static_cast<ID3D11Texture2D*>(ptr)}
        return tex;
    }

    BackBuffer::BackBuffer(ID3D11Device& device, wrl::ComPtr<ID3D11Texture2D> tex)
        : tex_{std::move(tex)}
    {
        TryHR(device.CreateRenderTargetView(tex_.Get(), {}, rtView_.ReleaseAndGetAddressOf()));
        SetName(Ref(tex_), "BackBuffer"sv);
    }

    void BackBuffer::Clear(ID3D11DeviceContext& context, DirectX::XMVECTOR color)
    {
        DirectX::XMFLOAT4 colorFloats;
        DirectX::XMStoreFloat4(&colorFloats, color);
        context.ClearRenderTargetView(rtView_.Get(), reinterpret_cast<const float*>(&colorFloats));
    }

    void BackBuffer::Reset() noexcept
    {
        rtView_.Reset();
        tex_.Reset();
    }

    DepthStencil::DepthStencil(ID3D11Device & device, std::uint32_t width, std::uint32_t height, DxgiFormat format)
    {
        CD3D11_TEXTURE2D_DESC desc{
            static_cast<DXGI_FORMAT>(format),
            width,
            height
        };
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TryHR(device.CreateTexture2D(&desc, {}, tex_.GetAddressOf()));
        SetName(Ref(tex_), "Depth Buffer");
        D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc{};
        depthDesc.Format = static_cast<DXGI_FORMAT>(format);
        depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        TryHR(device.CreateDepthStencilView(tex_.Get(), &depthDesc, view_.GetAddressOf()));
    }

    void DepthStencil::ClearDepth(ID3D11DeviceContext & context, float depth)
    {
        context.ClearDepthStencilView(&View(), D3D11_CLEAR_DEPTH, depth, {});
    }

    void DepthStencil::ClearStencil(ID3D11DeviceContext & context, std::uint8_t stencil)
    {
        context.ClearDepthStencilView(&View(), D3D11_CLEAR_STENCIL, {}, stencil);
    }

    void DepthStencil::ClearBoth(ID3D11DeviceContext& context, float depth, std::uint8_t stencil)
    {
        context.ClearDepthStencilView(&View(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void DepthStencil::Reset()
    {
        view_.Reset();
        tex_.Reset();
    }

    void DependentGraphics::Bind(ID3D11DeviceContext& context3D)
    {
        auto& backBuffer = SwapChain_.Front();
        ID3D11RenderTargetView* views[] = { &backBuffer.RtView() };
        context3D.OMSetRenderTargets(gsl::narrow<UINT>(std::size(views)), views, &DepthStencil_.View());
    }
}