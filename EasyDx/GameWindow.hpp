#pragma once

#include "WinDecl.hpp"
#include "DXDef.hpp"

namespace dx
{
    struct EventLoop;

    class GameWindow : Noncopyable
    {
    public:
        using Win32WindowHandle = HWND__* ;
        friend struct MessageDispatcher;

        explicit GameWindow(EventLoop& loop, const wchar_t* title = L"EasyDX",
            std::optional<std::uint32_t> width = std::nullopt,
            std::optional<std::uint32_t> height = std::nullopt);

        void Show();
        void Relocate(const Rect& rect);
        Win32WindowHandle NativeHandle() const noexcept;
        std::uint32_t GetWidth() const noexcept;
        std::uint32_t GetHeight() const noexcept;
        float GetDpiX() const noexcept;
        float GetDpiY() const noexcept;
        ~GameWindow();

    private:
        void PrepareForDpiChanging(std::uint32_t dpiX, std::uint32_t dpiY, const Rect& newWindowRect);
        void InitializeDpi();

        Win32WindowHandle windowHandle_;
        std::uint32_t width_{}, height_{};
        std::uint32_t dpiX_{}, dpiY_{};

        //TODO: remove this, too.
        //wrl::ComPtr<ID2D1Bitmap1> targetBitmap_;
    };
}