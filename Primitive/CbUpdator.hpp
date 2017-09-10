#pragma once

#include <EasyDx/BehaviorFwd.hpp>

namespace dx
{
    class Camera;
}

struct CbPerObjectUpdator
{
    dx::Camera& Camera;
    void operator()(const dx::GameObject& object, const dx::UpdateArgs& args);
};