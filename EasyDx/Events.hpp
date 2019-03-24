#pragma once

#include <unordered_map>
#include <variant>
#include <chrono>
#include "InputSystem.hpp"
#include "Misc.hpp"

namespace dx
{
    class GameWindow;

    struct IEventHandle
    {
        virtual ~IEventHandle();
    };

    template<typename EventT>
    struct EventHandle : IEventHandle
    {
      public:
        EventHandle(EventT& event, std::uint32_t handle) : event_{event}, handle_{handle} {}

        ~EventHandle() override;

      private:
        std::reference_wrapper<EventT> event_;
        std::uint32_t handle_;
    };

    enum class ControlFlow
    {
        kContinue,
        kBreak
    };

    // TODO: enable_share_from_this & weak_ptr?
    template<typename... Args>
    class Events
    {
      public:
        using Handler = std::function<ControlFlow(Args...)>;
        using HandleType = EventHandle<Events>;

        Events() = default;

        Events(const Events&) = delete;
        Events& operator=(const Events&) = delete;

        // TODO: 支持少参数的情况。
        template<typename Func>
        [[nodiscard]] std::unique_ptr<IEventHandle> Add(Func func) {
            const auto handle = AllocateHandle();
            using RetType = std::invoke_result_t<Func, Args...>;
            if constexpr (std::is_same_v<RetType, void>)
            {
                handlers_.insert({handle, [func = std::move(func)](Args&&... args) {
                                      func(exforward(args)...);
                                      return ControlFlow::kContinue;
                                  }});
            }
            else if constexpr (std::is_same_v<RetType, ControlFlow>)
            {
                handlers_.insert({handle, std::move(func)});
            }
            else
            {
                static_assert(always_false<Func>::value);
            }
            return std::make_unique<HandleType>(*this, handle);
        }

        void Remove(std::uint32_t handle)
        {
            handlers_.erase(handle);
        }

        void operator()(Args... args) const
        {
            for (auto& kv : handlers_)
            {
                if (kv.second(args...) == ControlFlow::kBreak)
                {
                    break;
                }
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

    template<typename WindowArg>
    using WindowEvents = Events<GameWindow*, WindowArg>;

    enum class MouseButton
    {
        kLeft,
        kRight,
        kMiddle
    };

    struct KeyStates
    {
        std::uint32_t KeyStateBits;

        bool Control() const noexcept;
        bool Left() const noexcept;
        bool Middle() const noexcept;
        bool Right() const noexcept;
        bool Shift() const noexcept;
    };

    //应该在 Args 中携带尽可能多的按键信息，以防在执行 event 前按键状态发生变化。
    struct MouseEventArgs
    {
        Point Position;
        ElementState State;
        MouseButton Button;
        KeyStates KeyStates_;
    };

    struct CursorMoved
    {
        Point Position;
        KeyStates KeyStates_;
    };

    struct KeyEventArgs
    {
        std::uint32_t Key;
        ElementState State;
        KeyStates KeyStates_;
    };

    struct ResizeEventArgs
    {
        Size NewSize;
    };

    struct DpiChangedEventArgs
    {
        std::uint32_t NewDpiX, NewDpiY;
        IntRect NewRect;
    };

    struct QuitEventArgs
    {};

    struct IdleEventArgs
    {
        // duration since last idle.
        // Duration duration;
    };

#define DefWindowEvent(name, arg) using name = WindowEvents<arg&>

    DefWindowEvent(WindowResizeEvent, ResizeEventArgs);
    DefWindowEvent(DpiChangedEvent, DpiChangedEventArgs);

    using WindowEventArgs = std::variant<KeyEventArgs, MouseEventArgs, CursorMoved, ResizeEventArgs,
                                         DpiChangedEventArgs, QuitEventArgs, IdleEventArgs>;

    struct WindowEventArgsPack
    {
        GameWindow* Window;
        WindowEventArgs Arg;
    };

#undef DefWindowEvent
} // namespace dx