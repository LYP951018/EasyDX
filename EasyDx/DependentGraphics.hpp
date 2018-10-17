#pragma once

#include <DirectXMath.h>

namespace dx
{
    class GameWindow;

    struct SwapChainOptions
    {
        std::uint32_t CountOfBackBuffers = 2;
        DxgiFormat BackBufferFormat = DxgiFormat::B8G8R8A8UNorm;
        DxgiFormat DepthBufferFormat = DxgiFormat::Depth24UNormStencilS8UInt;
        bool Windowed = true;
    };

    wrl::ComPtr<ID3D11Texture2D> GetBuffer(IDXGISwapChain& swapChain, std::uint32_t index = 0);

    struct BackBuffer : Noncopyable
    {
        BackBuffer() = default;
        BackBuffer(ID3D11Device& device, wrl::ComPtr<ID3D11Texture2D> tex);
        // TODO: a better color type?
        void Clear(ID3D11DeviceContext& context, DirectX::XMVECTOR color);
        auto AsTuple() const { return std::tie(Ref(tex_), Ref(rtView_)); }
        void Reset() noexcept;
        ID3D11RenderTargetView& RtView() const noexcept { return Ref(rtView_); }

      private:
        wrl::ComPtr<ID3D11Texture2D> tex_;
        wrl::ComPtr<ID3D11RenderTargetView> rtView_;
    };

    struct SwapChain : Noncopyable
    {
      public:
        SwapChain(ID3D11Device& device, const GameWindow& window, const SwapChainOptions& options);
        SwapChain(SwapChain&& rhs);
        ~SwapChain();

        BackBuffer& Front() const;
        void Present();
        void Reset();
        void Resize(ID3D11Device& device, std::uint32_t height, std::uint32_t width);

      private:
        wrl::ComPtr<IDXGISwapChain> swapChain_;
        //FIXME
        mutable BackBuffer backBuffer_;
        // options used for resizing.
        const SwapChainOptions options_;

        void UpdateBuffers(ID3D11Device& device);
    };

    struct DepthStencil
    {
        DepthStencil(ID3D11Device& device, std::uint32_t width, std::uint32_t height,
                     DxgiFormat format = DxgiFormat::Depth24UNormStencilS8UInt);

        void ClearDepth(ID3D11DeviceContext& context, float depth);
        void ClearStencil(ID3D11DeviceContext& context, std::uint8_t stencil);
        void ClearBoth(ID3D11DeviceContext& context, float depth = 1.0f, std::uint8_t stencil = 0);
        void Reset();

        auto AsTuple() const { return std::tie(Ref(tex_), Ref(view_)); }

        ID3D11DepthStencilView& View() const noexcept { return Ref(view_); }

      private:
        wrl::ComPtr<ID3D11Texture2D> tex_;
        wrl::ComPtr<ID3D11DepthStencilView> view_;
    };

    struct DependentGraphics : Noncopyable
    {
        SwapChain SwapChain_;
        DepthStencil DepthStencil_;

        DependentGraphics(SwapChain swapChain, DepthStencil depthStencil)
            : SwapChain_{std::move(swapChain)}, DepthStencil_{std::move(depthStencil)}
        {}

        void Bind(ID3D11DeviceContext& context3D) const;
    };
} // namespace dx
