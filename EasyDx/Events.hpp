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

    template<typename Arg>
    class Event;

    template<typename EventT>
    struct EventHandle
    {
    public:
        EventHandle(EventT& event, std::uint32_t handle)
            : event_{event}, handle_{handle}
        {}

        ~EventHandle();

    private:
        std::reference_wrapper<EventT> event_;
        std::uint32_t handle_;
    };

    template<typename Args>
    class Event
    {
        static_assert(std::is_base_of_v<EventArgs, std::remove_reference_t<Args>>, "Args should be base of EventArgs");
    public:
        using Handler = std::function<void(Args)>;
        using HandleType = EventHandle<Event<Args>>;

        Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        HandleType Add(Handler handler)
        {
            const auto handle = AllocateHandle();
            handlers_.insert({handle, std::move(handler) });
            return { *this, handle };
        }

        void Remove(std::uint32_t handle)
        {
            handlers_.erase(handle);
        }

        void operator()(Args args) const
        {
            for (auto& kv : handlers_)
            {
                kv.second(std::forward<Args>(args));
            }
        }

    private:
        std::uint32_t AllocateHandle() noexcept
        {
            return std::exchange(currentHandle_, currentHandle_ + 1);
        }

        std::unordered_map<std::uint32_t, Handler> handlers_;
        std::uint32_t currentHandle_ = {};
    };

    template<typename EventT>
    EventHandle<EventT>::~EventHandle()
    {
        event_.get().Remove(handle_);
    }

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

    using WindowResizeEventHandle = dx::EventHandle<dx::WindowResizeEvent>;
}