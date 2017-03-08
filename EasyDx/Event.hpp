#pragma once

#include <functional>
#include <unordered_map>

namespace dx
{
    template<typename...>
    class Event;

    template<typename R, typename... Args>
    class Event<R(Args...)>
    {
    public:
        using Handle = std::uint32_t;
        using Handler = std::function<R(Args...)>;

        [[maybe_unused]]
        Handle Add(Handler handler)
        {
            handlers_.insert({ currentHandle_, std::move(handler) });
            return std::exchange(currentHandle_, currentHandle_ + 1);
        }

        void Remove(Handle handle)
        {
            handlers_.erase(handle);
        }

        void operator()(Args... args)
        {
            for (auto& kv : handlers_)
            {
                kv.second(std::forward<Args>(args)...);
            }
        }

    private:
        std::unordered_map<Handle, Handler> handlers_;
        std::uint32_t currentHandle_ = {};
    };
    
}