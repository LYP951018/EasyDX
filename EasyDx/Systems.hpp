#pragma once

#include "BehaviorFwd.hpp"

namespace dx
{
    struct Renderable;

    void RenderSystem(ID3D11DeviceContext& context, const GameObject& object);
    void BehaviorSystem(GameObject& object, const UpdateArgs& args);
}