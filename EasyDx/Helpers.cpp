#include "pch.hpp"
#include "Helpers.hpp"
#include "Camera.hpp"

namespace dx
{
    void BasicCbUpdator::Update(GameObject& object, const UpdateArgs &)
    {
        auto renderable = GetComponent<Renderable>(object);
        auto cb = std::dynamic_pointer_cast<cb::Basic>(renderable->Cb);
        auto& data = cb->Data();
        const auto world = ComputeWorld(object.Transform);
        data.World = world;
        data.WorldInvTranspose = DirectX::XMMatrixInverse({}, DirectX::XMMatrixTranspose(world));
        data.WorldViewProj = world * Camera_.GetView() * Camera_.GetProjection();
    }
}

