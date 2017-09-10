#pragma once

#include <functional>

namespace dx
{
    class GameObject;
    struct UpdateArgs;

    struct Behavior
    {
        enum Time
        {
            kPreCbUpdate = 0,
            kCbUpdate = 1,
            kPostCbUpdate = 2
        };

        Behavior(std::uint32_t priority) noexcept
            : time_{priority}
        {}

        virtual void Update(GameObject&, const UpdateArgs&) = 0;

        std::uint32_t GetExeTime() const noexcept
        {
            return time_;
        }

    private:
        std::uint32_t time_;
    };

    struct FuncBehavior : Behavior
    {
        template<typename F>
        FuncBehavior(F&& f, std::uint32_t priority = Behavior::kPreCbUpdate)
            : func_{ std::forward<F>(f) },
            Behavior{ priority }
        {}

        void Update(GameObject& object, const UpdateArgs& args) override;

    private:
        std::function<void(GameObject&, const UpdateArgs&)> func_;
    };
}