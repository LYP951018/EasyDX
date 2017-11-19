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

    struct IEventHandle 
    {
        virtual ~IEventHandle();
    };

    template<typename EventT>
    struct EventHandle : IEventHandle
    {
    public:
        EventHandle(EventT& event, std::uint32_t handle)
            : event_{event}, handle_{handle}
        {}

        ~EventHandle() override;

    private:
        std::reference_wrapper<EventT> event_;
        std::uint32_t handle_;
    };

    //TODO: enable_share_from_this & weak_ptr?
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

        [[nodiscard]]
        std::unique_ptr<IEventHandle> Add(Handler handler)
        {
            const auto handle = AllocateHandle();
            handlers_.insert({handle, std::move(handler) });
            return std::make_unique<HandleType>(*this, handle);
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

    //应该在 Args 中携带尽可能多的按键信息，以防在执行 event 前按键状态发生变化。
    struct MouseEventArgs : EventArgs
    {
        Point Position;
        //MouseButton Button;
        std::uint32_t KeyStates;

        bool Control() const noexcept;
        bool Left() const noexcept;
        bool Middle() const noexcept;
        bool Right() const noexcept;
        bool Shift() const noexcept;
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
    using MouseMoveEvent = Event<MouseEventArgs&>;
    using WindowResizeEvent = Event<ResizeEventArgs&>;
    using DpiChangedEvent = Event<DpiChangedEventArgs&>;

    using WindowResizeEventHandle = dx::EventHandle<dx::WindowResizeEvent>;
}