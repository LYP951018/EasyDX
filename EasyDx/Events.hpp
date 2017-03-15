#pragma once

#include <functional>
#include <unordered_map>
#include <cstdint>
#include <type_traits>
#include "Misc.hpp"

namespace dx
{
    class GameWindow;

    struct EventArgs
    {
        bool IsHandled = false;
        GameWindow* Window = {};
    };

    
    enum class EventHandle: std::uint32_t
    {};

    template<typename Args>
    class Event
    {
        static_assert(std::is_base_of_v<EventArgs, std::remove_reference_t<Args>>, "Args should be base of EventArgs");
    public:
        
        using Handler = std::function<void(Args)>;

        Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        EventHandle Add(Handler handler)
        {
            const auto handle = AllocateHandle();
            handlers_.insert({handle, std::move(handler) });
            return handle;
        }

        void Remove(EventHandle handle)
        {
            handlers_.erase(handle);
        }

        void operator()(Args args)
        {
            for (auto& kv : handlers_)
            {
                kv.second(std::forward<Args>(args));
            }
        }

    private:
        EventHandle AllocateHandle() noexcept
        {
            return static_cast<EventHandle>(std::exchange(currentHandle_, currentHandle_ + 1));
        }

        std::unordered_map<EventHandle, Handler> handlers_;
        std::underlying_type_t<EventHandle> currentHandle_ = {};
    };

    struct KeyEventArgs : EventArgs
    {
        std::uint32_t Key;
    };

    enum class MouseButton
    {
        kLeft, kRight, kMiddle
    };

    struct MouseEventArgs : EventArgs
    {
        MouseButton Button;
        Point Position;
    };

    struct MouseMoveEventArgs : EventArgs
    {
        Point From, To;
    };

    struct ResizeEventArgs : EventArgs
    {
        Size NewSize;
    };

    struct DpiChangedEventArgs : EventArgs
    {
        std::uint32_t NewDpiX, NewDpiY;
    };

    using KeyDownEvent = Event<KeyEventArgs&>;
    using KeyUpEvent = Event<KeyEventArgs&>;
    using MouseDownEvent = Event<MouseEventArgs&>;
    using MouseUpEvent = Event<MouseEventArgs&>;
    using WindowResizeEvent = Event<ResizeEventArgs&>;
    using DpiChangedEvent = Event<DpiChangedEventArgs&>;
}