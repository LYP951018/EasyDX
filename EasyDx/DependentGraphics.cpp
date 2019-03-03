#include "pch.hpp"
#include "DependentGraphics.hpp"
#include "GameWindow.hpp"
#include "D3DHelpers.hpp"
#include "GraphicsDevices.hpp"
#include "Misc.hpp"
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite_1.h>

using namespace std::literals;

namespace dx
{
    IndependentGraphics::IndependentGraphics()
    {
        std::tie(device3D_, context3D_) = MakeDevice3D();
        TryHR(device3D_->QueryInterface(dxgiDevice_.ReleaseAndGetAddressOf()));
#if _DEBUG
        TryHR(device3D_->QueryInterface(d3dDebug_.ReleaseAndGetAddressOf()));
#else
        device3D_->QueryInterface(d3dDebug_.ReleaseAndGetAddressOf());
#endif

        D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        TryHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options,
                                fanctory2D_.ReleaseAndGetAddressOf()));
        TryHR(fanctory2D_->CreateDevice(dxgiDevice_.Get(), device2D_.ReleaseAndGetAddressOf()));
        TryHR(device2D_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                             context2D_.ReleaseAndGetAddressOf()));

        TryHR(
            DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1),
                                reinterpret_cast<IUnknown**>(dwFactory_.ReleaseAndGetAddressOf())));
    }

    namespace Internal
    {
        template<typename T>
        wrl::ComPtr<T> GetParent(IDXGIObject& object)
        {
            void* ptr;
            TryHR(object.GetParent(__uuidof(T), &ptr));
            wrl::ComPtr<T> result;
            result.Attach(static_cast<T*>(ptr));
            return result;
        }
    } // namespace Internal

    wrl::ComPtr<ID3D11Texture2D> GetBuffer(IDXGISwapChain& swapChain, std::uint32_t index)
    {
        wrl::ComPtr<ID3D11Texture2D> tex;
        void* ptr;
        TryHR(swapChain.GetBuffer(gsl::narrow<UINT>(index), __uuidof(ID3D11Texture2D), &ptr));
        tex.Attach(static_cast<ID3D11Texture2D*>(ptr));
        // leak: return wrl::ComPtr<ID3D11Texture2D>{static_cast<ID3D11Texture2D*>(ptr)}
        return tex;
    }

    DXGI_SWAP_CHAIN_DESC DefaultSwapChainDescFromWindowHandle(void* windowHandle)
    {
        return SwapChainDescBuilder{}.WindowHandle(windowHandle).Build();
    }

    SwapChain::SwapChain(ID3D11Device& device, const DXGI_SWAP_CHAIN_DESC& options)
    {
        wrl::ComPtr<IDXGIDevice1> dxgiDevice;
        TryHR(device.QueryInterface(dxgiDevice.GetAddressOf()));
        const auto adapter = Internal::GetParent<IDXGIAdapter1>(Ref(dxgiDevice));
        const auto factory = Internal::GetParent<IDXGIFactory1>(Ref(adapter));
        // CreateSwapChain 接受的 Desc 不是 const，IDXGIFactory2::CreateSwapChainForHwnd 接受的 desc
        // 是 const
        // TODO: switch to CreateSwapChainForHwnd
        auto fuckMsCopy = options;
        TryHR(factory->CreateSwapChain(&device, &fuckMsCopy, swapChain_.GetAddressOf()));
        UpdateBuffers(device);
    }

    SwapChain::~SwapChain() {}

    ID3D11Texture2D& SwapChain::Front() const { return Ref(m_backBuffer); }

    void SwapChain::Present() { TryHR(swapChain_->Present(0, 0)); }

    void SwapChain::Reset() { m_backBuffer.Reset(); }

    // https://stackoverflow.com/questions/28095798/how-to-change-window-size-in-directx-11-desktop-application
    void SwapChain::Resize(ID3D11Device& device, const DXGI_SWAP_CHAIN_DESC& options)
    {
        TryHR(swapChain_->ResizeBuffers(options.BufferCount,
                                        gsl::narrow<UINT>(options.BufferDesc.Width),
                                        gsl::narrow<UINT>(options.BufferDesc.Height),
                                        options.BufferDesc.Format, options.Flags));
        UpdateBuffers(device);
    }

    Size SwapChain::BufferSize() const
    {
        auto& bufferTex = Front();
        D3D11_TEXTURE2D_DESC desc{};
        bufferTex.GetDesc(&desc);
        return Size{desc.Width, desc.Height};
    }

    // FIXME: why device here?
    void SwapChain::UpdateBuffers(ID3D11Device&)
    {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/mt427784%28v=vs.85%29.aspx
        // In Direct3D 11, applications could call GetBuffer(0, ��) only once.Every call to Present
        // implicitly changed the resource identity of the returned interface.
        m_backBuffer = GetBuffer(Ref(swapChain_), 0);
    }

    DepthStencil::DepthStencil(ID3D11Device& device, const Size& size, DxgiFormat format)
    {
        CD3D11_TEXTURE2D_DESC desc{static_cast<DXGI_FORMAT>(format), size.Width, size.Height};
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TryHR(device.CreateTexture2D(&desc, {}, tex_.GetAddressOf()));
        SetName(Ref(tex_), "Depth Buffer");
        D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc{};
        depthDesc.Format = static_cast<DXGI_FORMAT>(format);
        depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        TryHR(device.CreateDepthStencilView(tex_.Get(), &depthDesc, view_.GetAddressOf()));
    }

    void DepthStencil::ClearDepth(ID3D11DeviceContext& context, float depth)
    {
        context.ClearDepthStencilView(View(), D3D11_CLEAR_DEPTH, depth, {});
    }

    void DepthStencil::ClearStencil(ID3D11DeviceContext& context, std::uint8_t stencil)
    {
        context.ClearDepthStencilView(View(), D3D11_CLEAR_STENCIL, {}, stencil);
    }

    void DepthStencil::ClearBoth(ID3D11DeviceContext& context, float depth, std::uint8_t stencil)
    {
        context.ClearDepthStencilView(View(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
                                      stencil);
    }

    void DepthStencil::Reset()
    {
        view_.Reset();
        tex_.Reset();
    }

    SwapChainDescBuilder::SwapChainDescBuilder()
    {
        m_desc = {};
        m_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_desc.SampleDesc.Count = 1;
        m_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_desc.BufferCount = 1;
        m_desc.Windowed = TRUE;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::Size(std::uint32_t width, std::uint32_t height)
    {
        auto& bufferDesc = m_desc.BufferDesc;
        bufferDesc.Width = width;
        bufferDesc.Height = height;
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::WindowHandle(void* handle)
    {
        m_desc.OutputWindow = static_cast<HWND>(handle);
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::BufferCount(std::uint32_t count)
    {
        m_desc.BufferCount = static_cast<UINT>(count);
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::Windowed()
    {
        m_desc.Windowed = TRUE;
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::FullScreen()
    {
        m_desc.Windowed = FALSE;
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::DisplayFormat(DxgiFormat format)
    {
        m_desc.BufferDesc.Format = static_cast<DXGI_FORMAT>(format);
        return *this;
    }

    SwapChainDescBuilder& SwapChainDescBuilder::SwapEffect(DxgiSwapEffect effect)
    {
        m_desc.SwapEffect = static_cast<DXGI_SWAP_EFFECT>(effect);
        return *this;
    }

    const DXGI_SWAP_CHAIN_DESC& SwapChainDescBuilder::Build() { return m_desc; }

    void GlobalGraphicsContext::ClearDepth(float depth)
    {
        GetDepthStencil().ClearDepth(Context3D(), depth);
    }

    void GlobalGraphicsContext::ClearStencil(std::uint8_t stencil)
    {
        GetDepthStencil().ClearStencil(Context3D(), stencil);
    }

    void GlobalGraphicsContext::ClearBoth(float depth, std::uint8_t stencil)
    {
        GetDepthStencil().ClearBoth(Context3D(), depth, stencil);
    }

    void GlobalGraphicsContext::OnResize(const Size& newSize)
    {
        OnResize(Device3D(), Context3D(), newSize);
    }

    void GlobalGraphicsContext::ClearMainRt(DirectX::XMVECTOR color)
    {
        ClearMainRt(Context3D(), color);
    }

    DependentGraphics::DependentGraphics(ID3D11Device& device3D, const DXGI_SWAP_CHAIN_DESC& desc)
        : SwapChainDesc{desc}, m_swapChain{device3D, desc}
    {
        // hack
        RecreateDepthStencil(device3D);
        CreateMainRt(device3D);
    }

    void DependentGraphics::RecreateDepthStencil(ID3D11Device& device3D)
    {
        m_depthStencil = DepthStencil{device3D, m_swapChain.BufferSize()};
    }

    void DependentGraphics::ClearMainRt(ID3D11DeviceContext& context3D, DirectX::XMVECTOR color)
    {
        DirectX::XMFLOAT4 colorFloats;
        DirectX::XMStoreFloat4(&colorFloats, color);
        context3D.ClearRenderTargetView(m_mainRt.Get(),
                                        reinterpret_cast<const float*>(&colorFloats));
    }

    void DependentGraphics::OnResize(ID3D11Device& device3D, ID3D11DeviceContext& context3D,
                                     [[maybe_unused]] const Size& newSize)
    {
        // https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/d3d10-graphics-programming-guide-dxgi#handling-window-resizing
        context3D.OMSetRenderTargets(0, nullptr, nullptr);
        Ensures(m_mainRt.Reset() == 0);
        m_depthStencil.Reset();
        auto& swapChain = GetSwapChain();
        swapChain.Reset();
        swapChain.Resize(device3D, SwapChainDesc);
        RecreateDepthStencil(device3D);
        CreateMainRt(device3D);
    }

    void DependentGraphics::CreateMainRt(ID3D11Device& device3D)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtDesc{};
        rtDesc.Format = SwapChainDesc.BufferDesc.Format;
        rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        TryHR(device3D.CreateRenderTargetView(&GetSwapChain().Front(), &rtDesc,
                                              m_mainRt.ReleaseAndGetAddressOf()));
    }

} // namespace dx