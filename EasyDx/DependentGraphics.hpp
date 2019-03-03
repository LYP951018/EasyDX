#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

namespace dx
{
    class IndependentGraphics
    {
      private:
        wrl::ComPtr<ID3D11Device> device3D_;
        wrl::ComPtr<ID3D11DeviceContext> context3D_;
        wrl::ComPtr<ID2D1Factory1> fanctory2D_;
        wrl::ComPtr<ID2D1Device> device2D_;
        wrl::ComPtr<ID2D1DeviceContext> context2D_;
        wrl::ComPtr<IDXGIDevice> dxgiDevice_;
        wrl::ComPtr<IDWriteFactory1> dwFactory_;
        wrl::ComPtr<ID3D11Debug> d3dDebug_;

      public:
        IndependentGraphics();

        DELETE_COPY(IndependentGraphics)
        IndependentGraphics(IndependentGraphics&&) = default;

        ID3D11Device& Device3D() const { return Ref(device3D_); }
        wrl::ComPtr<ID3D11Device> SharedDevice3D() const { return device3D_; }
        ID3D11DeviceContext& Context3D() const { return Ref(context3D_); }
        ID2D1Factory1& Factory2D() const { return Ref(fanctory2D_); }
        ID2D1Device& Device2D() const { return Ref(device2D_); }
        ID2D1DeviceContext& Context2D() const { return Ref(context2D_); }
        IDXGIDevice& DxgiDevice() const { return Ref(dxgiDevice_); }
        IDWriteFactory1& DwFactory() const { return Ref(dwFactory_); }
        ID3D11Debug& D3DDebug() const { return Ref(d3dDebug_); }

        ~IndependentGraphics();
    };

    wrl::ComPtr<ID3D11Texture2D> GetBuffer(IDXGISwapChain& swapChain, std::uint32_t index = 0);

    struct Size;

    struct SwapChain : Noncopyable
    {
      public:
        SwapChain(ID3D11Device& device, const DXGI_SWAP_CHAIN_DESC& options);
        DEFAULT_MOVE(SwapChain)
        ~SwapChain();

        ID3D11Texture2D& Front() const;
        void Present();
        void Reset();
        void Resize(ID3D11Device& device, const DXGI_SWAP_CHAIN_DESC& options);
        Size BufferSize() const;

      private:
        wrl::ComPtr<IDXGISwapChain> swapChain_;
        wrl::ComPtr<ID3D11Texture2D> m_backBuffer;
        void UpdateBuffers(ID3D11Device& device);
    };

    struct SwapChainDescBuilder
    {
        SwapChainDescBuilder();
        SwapChainDescBuilder& Size(std::uint32_t width, std::uint32_t height);
        SwapChainDescBuilder& WindowHandle(void* handle);
        SwapChainDescBuilder& BufferCount(std::uint32_t count);
        SwapChainDescBuilder& Windowed();
        SwapChainDescBuilder& FullScreen();
        SwapChainDescBuilder& DisplayFormat(DxgiFormat format);
        SwapChainDescBuilder& SwapEffect(DxgiSwapEffect effect);
        const DXGI_SWAP_CHAIN_DESC& Build();

      private:
        DXGI_SWAP_CHAIN_DESC m_desc;
    };

    DXGI_SWAP_CHAIN_DESC DefaultSwapChainDescFromWindowHandle(void* windowHandle);

    struct DepthStencil
    {
        DepthStencil() = default;
        DepthStencil(ID3D11Device& device, const Size& size,
                     DxgiFormat format = DxgiFormat::Depth24UNormStencilS8UInt);

        DELETE_COPY(DepthStencil)
        DEFAULT_MOVE(DepthStencil)

        void ClearDepth(ID3D11DeviceContext& context, float depth);
        void ClearStencil(ID3D11DeviceContext& context, std::uint8_t stencil);
        void ClearBoth(ID3D11DeviceContext& context, float depth = 1.0f, std::uint8_t stencil = 0);
        void Reset();

        auto AsTuple() const { return std::tie(Ref(tex_), Ref(view_)); }

        ID3D11DepthStencilView* View() const noexcept { return view_.Get(); }
        ID3D11Texture2D& Tex() const { return Ref(tex_); }

      private:
        wrl::ComPtr<ID3D11Texture2D> tex_;
        wrl::ComPtr<ID3D11DepthStencilView> view_;
    };


    class DependentGraphics
    {
    public:
        DXGI_SWAP_CHAIN_DESC SwapChainDesc;

        DependentGraphics(ID3D11Device& device3D, const DXGI_SWAP_CHAIN_DESC& desc);

        SwapChain& GetSwapChain() { return m_swapChain; }
        DepthStencil& GetDepthStencil() { return m_depthStencil; }
        const DepthStencil& GetDepthStencil() const { return m_depthStencil; }
        ID3D11RenderTargetView* MainRt() const { return m_mainRt.Get(); }
        void ClearMainRt(ID3D11DeviceContext& context3D, DirectX::XMVECTOR color);

        DELETE_COPY(DependentGraphics)
        DEFAULT_MOVE(DependentGraphics)

      protected:
        void OnResize(ID3D11Device& device3D, ID3D11DeviceContext& context3D, const Size& newSize);

      private:
        void CreateMainRt(ID3D11Device& device3D);
        void RecreateDepthStencil(ID3D11Device& device3D);

        SwapChain m_swapChain;
        DepthStencil m_depthStencil;
        wrl::ComPtr<ID3D11RenderTargetView> m_mainRt;
    };

    class GlobalGraphicsContext : IndependentGraphics, DependentGraphics
    {
      public:
          //TODO: 这个构造函数做的事情有点多 = =
        GlobalGraphicsContext(const DXGI_SWAP_CHAIN_DESC& swapChainDesc)
            : DependentGraphics{Device3D(), swapChainDesc}
        {}

        void ClearDepth(float depth);
        void ClearStencil(std::uint8_t stencil);
        void ClearBoth(float depth = 1.0f, std::uint8_t stencil = 0);
        void ClearMainRt(DirectX::XMVECTOR color);
        //TODO
        void SetResolution(std::uint32_t width, std::uint32_t height);
        using DependentGraphics::GetDepthStencil;
        using DependentGraphics::GetSwapChain;
        using DependentGraphics::MainRt;
        using IndependentGraphics::Context3D;
        using IndependentGraphics::Device3D;

      private:
        friend class Game;
        using DependentGraphics::ClearMainRt;
        using DependentGraphics::OnResize;
        void OnResize(const Size& newSize);
    };
} // namespace dx
