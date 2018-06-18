#include "pch.hpp"
#include "GameWindow.hpp"
#include "EventLoop.hpp"
#include "Misc.hpp"
#include <gsl/gsl_assert>
#define NOMINMAX
#include <Windows.h>


namespace dx
{
    ::HINSTANCE GetInstanceHandle()
    {
        const static ::HINSTANCE instance = ::GetModuleHandle(nullptr);
        Ensures(instance != nullptr);
        return instance;
    }

    static_assert(std::is_same_v<GameWindow::Win32WindowHandle, ::HWND>);

    const wchar_t* kWindowClassName = L"EasyDxWindow";

    GameWindow::GameWindow(EventLoop& loop, const wchar_t* title,
        std::optional<std::uint32_t> width,
        std::optional<std::uint32_t> height)
    {
        ::WNDCLASSEX windowClass = {};
        windowClass.cbSize = sizeof(::WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = &EventLoop::WndProc;
        windowClass.hInstance = GetInstanceHandle();
        windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = kWindowClassName;
        windowClass.cbWndExtra = sizeof(std::intptr_t);
        ::RegisterClassEx(&windowClass);
        {
            std::mutex creationMutex;
            std::condition_variable creationCv;
            bool created = false;

            loop.ExecuteInThread([&] {
                windowHandle_ = ::CreateWindow(kWindowClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        width ? width.value() : CW_USEDEFAULT,
                        height ? height.value() : CW_USEDEFAULT,
                        nullptr, nullptr, GetInstanceHandle(), static_cast<void*>(this));
                {
                    std::lock_guard<std::mutex> lg{ creationMutex };
                    created = true;
                }
                creationCv.notify_one();
            });

            std::unique_lock<std::mutex> ul{ creationMutex };
            creationCv.wait(ul, [&] {return created; });
        }

        Ensures(windowHandle_ != nullptr);
        InitializeDpi();
    }

    void GameWindow::Show()
    {
        const auto handle = NativeHandle();
        ::ShowWindowAsync(handle, SW_SHOW);
        //::UpdateWindow(handle);
    }

    void GameWindow::Relocate(const IntRect& rect)
    {
        TryWin32(SetWindowPos(NativeHandle(),
            HWND_TOP,
            rect.LeftTopX,
            rect.LeftTopY,
            rect.Width,
            rect.Height,
            SWP_NOZORDER | SWP_NOACTIVATE));
    }

    auto GameWindow::NativeHandle() const noexcept -> Win32WindowHandle
    {
        return windowHandle_;
    }

    Size GameWindow::GetSize() const noexcept
    {
        return { width_, height_ };
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

    void GameWindow::PrepareForDpiChanging(std::uint32_t dpiX, std::uint32_t dpiY, const IntRect& newWindowRect)
    {
        dpiX_ = dpiX;
        dpiY_ = dpiY;
        Relocate(newWindowRect);
    }

    void GameWindow::OnResize(Size newSize)
    {
        width_ = newSize.Width;
        height_ = newSize.Height;
    }

    void GameWindow::InitializeDpi()
    {
        const auto dpi = GetDpiForWindow(NativeHandle());
        Ensures(dpi != 0);
        dpiX_ = static_cast<std::uint32_t>(dpi);
        dpiY_ = static_cast<std::uint32_t>(dpi);
    }
}
