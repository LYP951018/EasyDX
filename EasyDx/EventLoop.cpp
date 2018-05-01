#include "pch.hpp"
#include "EventLoop.hpp"
#include "GameWindow.hpp"
#include <windowsx.h>
#include <cassert>
#include <variant>

namespace dx
{
    void EventLoop::Run(std::function<void(WindowEventArgsPack)> callback)
    {
        assert(std::this_thread::get_id() == threadRunning_);
        
        while (true)
        {
            {
                std::unique_lock<std::mutex> lg{ swapLock_ };
                backEvents_.clear();
                swap(frontEvents_, backEvents_);
            }
            if (backEvents_.empty())
            {
                callback(WindowEventArgsPack{
                    nullptr,
                    IdleEventArgs{}
                });
            }
            else
            {
                for (auto& args : backEvents_)
                {
                    auto& arg = args.Arg;
                    if (std::holds_alternative<QuitEventArgs>(arg))
                    {
                        goto exit;
                    }
                    callback(args);
                    if (std::holds_alternative<ResizeEventArgs>(arg))
                    {
                        {
                            std::unique_lock<std::mutex> lg{ resizeLock_ };
                            resized_ = true;
                        }
                        resizeCv_.notify_one();
                    }
                }
            }
            
        }
    exit:
        if (pumpingThread_.joinable())
        {
            pumpingThread_.join();
        }
    }

    EventLoop& EventLoop::GetInstanceInCurrentThread()
    {
        static EventLoop loop;
        return loop;
    }

    void EventLoop::ExecuteInThread(std::function<void()> action)
    {
        std::lock_guard<std::mutex> lg{ queuedMutex_ };
        queuedCallbacks_.push(std::move(action));
    }

    EventLoop::EventLoop()
        : resized_ {},
        threadRunning_ {std::this_thread::get_id()}
    {
        pumpingThread_ = std::thread{ [&] {MessagePump(); } };
        pumpingThread_.detach();
    }

    void EventLoop::MessagePump()
    {
        ::MSG msg{};
        while (true)
        {
            if (::PeekMessage(&msg, nullptr, {}, {}, PM_REMOVE) != 0)
            {
                if (msg.message == WM_QUIT)
                {
                    PushEvent(nullptr, QuitEventArgs{});
                    break;
                }
                else
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }
            else
            {
                std::lock_guard<std::mutex> lg{ queuedMutex_ };
                if (queuedCallbacks_.empty())
                {
                    std::this_thread::yield();
                }
                else
                {
                    while (!queuedCallbacks_.empty())
                    {
                        auto& action = queuedCallbacks_.front();
                        action();
                        queuedCallbacks_.pop();
                    }
                }
            }
        }
    }

    void EventLoop::PushResizeEvent(GameWindow* window, ResizeEventArgs args)
    {
        PushEvent(window, std::move(args));
        std::unique_lock<std::mutex> lg{ resizeLock_ };
        resizeCv_.wait(lg, [&]() { return resized_; });
        resized_ = false;
    }

    Point PosFromLParam(LPARAM lParam) noexcept
    {
        return { static_cast<std::int32_t>(GET_X_LPARAM(lParam)),
            static_cast<std::int32_t>(GET_Y_LPARAM(lParam)) };
    }

    void ProcessMessage(GameWindow& window, std::uint32_t messageId, std::uintptr_t wParam, std::intptr_t lParam) noexcept
    {
        const auto MakeKeyStates = [&]() noexcept {
            return KeyStates{ static_cast<std::uint32_t>(wParam) };
        };
        const auto MakeMouseArgs = [&](MouseButton button, ElementState state) noexcept {
            return MouseEventArgs{
                PosFromLParam(lParam),
                state,
                button,
                MakeKeyStates()
            };
        };
        auto& eventLoop = EventLoop::GetInstanceInCurrentThread();
        const auto PushEvent = [&](auto arg)
        {
            eventLoop.PushEvent(&window, std::move(arg));
        };

        switch (messageId)
        {
        case WM_SIZE:
        {
            const auto newWidth = static_cast<std::uint32_t>(lParam & 0xFFFF);
            const auto newHeight = static_cast<std::uint32_t>(lParam >> 16);
            eventLoop.PushResizeEvent(&window, ResizeEventArgs{ newWidth, newHeight });
        }
        break;
        case WM_DESTROY:
            //TODO
            ::PostQuitMessage(0);
            break;
        case WM_DPICHANGED:
        {
            const auto newDpiX = static_cast<std::uint32_t>(LOWORD(wParam));
            const auto newDpiY = static_cast<std::uint32_t>(HIWORD(wParam));
            const auto rect = *reinterpret_cast<const RECT*>(lParam);
            const auto newRect = Rect::FromRECT(rect);
            PushEvent(DpiChangedEventArgs{ newDpiX, newDpiY, newRect });
        }
        break;
        case WM_KEYDOWN:
            PushEvent(KeyEventArgs{ static_cast<std::uint32_t>(wParam), ElementState::Pressed, MakeKeyStates() });
            break;
        case WM_KEYUP:
            PushEvent(KeyEventArgs{ static_cast<std::uint32_t>(wParam), ElementState::Released, MakeKeyStates() });
            break;
        case WM_LBUTTONDOWN:
            PushEvent(MakeMouseArgs(MouseButton::kLeft, ElementState::Pressed));
            break;
        case WM_RBUTTONDOWN:
            PushEvent(MakeMouseArgs(MouseButton::kRight, ElementState::Pressed));
            break;
        case WM_LBUTTONUP:
            PushEvent(MakeMouseArgs(MouseButton::kLeft, ElementState::Pressed));
            break;
        case WM_RBUTTONUP:
            PushEvent(MakeMouseArgs(MouseButton::kRight, ElementState::Released));
            break;
        case WM_MOUSEMOVE:
            PushEvent(CursorMoved{PosFromLParam(lParam), MakeKeyStates()});
            break;
        default:
            break;
        }
    }

    ::LRESULT __stdcall EventLoop::WndProc(::HWND windowHandle, ::UINT messageId, ::WPARAM wParam, ::LPARAM lParam) noexcept
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
                ProcessMessage(*window, static_cast<std::uint32_t>(messageId), static_cast<std::uintptr_t>(wParam), static_cast<std::intptr_t>(lParam));
            }
        }
        return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
    }
}
