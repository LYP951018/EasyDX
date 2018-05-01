#pragma once

#include "Events.hpp"
#include "MinimalWinDef.hpp" //include this for WndProc signature.
#include <queue>

namespace dx
{
    struct EventLoop : Noncopyable
    {
    public:
        //Run 只允许在 A 线程上调用。
        void Run(std::function<void(WindowEventArgsPack)> callback);
        static EventLoop& GetInstanceInCurrentThread();
        void ExecuteInThread(std::function<void()> action);

    private:
        EventLoop();
        friend class GameWindow;
        friend void ProcessMessage(GameWindow& window, std::uint32_t messageId, std::uintptr_t wParam, std::intptr_t lParam) noexcept;

        //PushEvent 只允许在 B 线程上调用。
        //TODO：限制传入 resize event？
        template<typename ArgT>
        void PushEvent(GameWindow* window, ArgT arg)
        {
            std::lock_guard<std::mutex> lg{ swapLock_ };
            frontEvents_.push_back(dx::WindowEventArgsPack{
                window, std::move(arg)
            });
            consumerCv_.notify_one();
        }

        void MessagePump();
        void PushResizeEvent(GameWindow* window, ResizeEventArgs args);
        static ::LRESULT __stdcall WndProc(::HWND windowHandle, ::UINT messageId, ::WPARAM wParam, ::LPARAM lParam) noexcept;

        bool resized_;
        std::vector<WindowEventArgsPack> frontEvents_;
        std::vector<WindowEventArgsPack> backEvents_;
        std::mutex swapLock_;
        std::mutex resizeLock_;
        std::condition_variable resizeCv_;
        std::condition_variable consumerCv_;
        std::thread::id threadRunning_;

        std::queue<std::function<void()>> queuedCallbacks_;
        std::mutex queuedMutex_;
        std::thread pumpingThread_;
    };
}