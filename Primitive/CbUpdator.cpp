#include "EasyDx/One.hpp"
#include "CbUpdator.hpp"
#include "Cb.hpp"
#include <DirectXMath.h>

using namespace DirectX;

void CbPerObjectUpdator::operator()(const dx::GameObject& object, const dx::UpdateArgs&)
{
    auto renderable = dx::GetComponent<dx::Pipeline_>(object);
    auto& smoothness = *dx::GetComponent<dx::Smoothness>(object);
    auto& cb = *std::dynamic_pointer_cast<CbPerObject>(renderable->Cb);
    auto& transform = object.Transform;
    auto& cpuCbPerObject = cb.Data();
    auto world = dx::ComputeWorld(transform);
    cpuCbPerObject.InvTransposeWorld = XMMatrixInverse({}, XMMatrixTranspose(world));
    cpuCbPerObject.World = world;
    cpuCbPerObject.WorldViewProjection = world * Camera.GetView() * Camera.GetProjection();
    cpuCbPerObject.MainMaterial.FromSmoothness(smoothness);
}
