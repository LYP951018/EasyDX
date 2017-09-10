#pragma once

#include <chrono>

namespace dx
{
    class GameObject;
    struct UpdateArgs;
}

struct Rotator
{
private:
    float angle_ = 0.f;
    std::chrono::milliseconds lastTime_{};

public:
    void operator()(dx::GameObject& object, const dx::UpdateArgs& args);
};