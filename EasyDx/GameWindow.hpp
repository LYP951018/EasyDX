#pragma once

#include "WinDecl.hpp"
#include "DXDef.hpp"
#include "Misc.hpp"

namespace dx
{
    struct EventLoop;

    class GameWindow : Noncopyable
    {
    public:
        using Win32WindowHandle = HWND__* ;
        friend struct MessageDispatcher;
        friend class Game;

        explicit GameWindow(EventLoop& loop, const wchar_t* title = L"EasyDX",
            std::optional<std::uint32_t> width = std::nullopt,
            std::optional<std::uint32_t> height = std::nullopt);

        void Show();
        void Relocate(const IntRect& rect);
        Win32WindowHandle NativeHandle() const noexcept;
        Size GetSize() const noexcept;
        float GetDpiX() const noexcept;
        float GetDpiY() const noexcept;
        ~GameWindow();

    private:
        void PrepareForDpiChanging(std::uint32_t dpiX, std::uint32_t dpiY, const IntRect& newWindowRect);
        void OnResize(Size newSize);
        void InitializeDpi();

        Win32WindowHandle windowHandle_;
        std::uint32_t width_{}, height_{};
        std::uint32_t dpiX_{}, dpiY_{};

        //TODO: remove this, too.
        //wrl::ComPtr<ID2D1Bitmap1> targetBitmap_;
    };
}