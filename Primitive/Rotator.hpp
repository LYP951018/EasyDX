#pragma once

#include <chrono>

namespace dx
{
    class GameObject;
    struct UpdateArgs;
    struct PointLight;
}

struct Rotator
{
private:
    float angle_ = 0.f;
    std::chrono::milliseconds lastTime_{};
    dx::PointLight& Light;

public:
    Rotator(dx::PointLight& light)
        : Light{ light }
    {}

    void operator()(dx::GameObject& object, const dx::UpdateArgs& args);
};