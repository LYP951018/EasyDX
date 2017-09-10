#include <EasyDx/One.hpp>
#include "Rotator.hpp"

void Rotator::operator()(dx::GameObject & object, const dx::UpdateArgs & args)
{
    using namespace std::chrono_literals;
    if (args.TotalTime - lastTime_ > 1s)
    {
        lastTime_ = args.TotalTime;
        if (angle_ > DirectX::XM_2PI)
            angle_ = 0.f;
        angle_ += DirectX::XM_PI / 120.f;
        auto& light = std::get<dx::PointLight>(dx::GetComponent<dx::LightComponent>(object)->Light_);
        auto pos = dx::MakePosition(light.Position);
        DirectX::XMStoreFloat3(&light.Position, DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&pos),
            DirectX::XMMatrixRotationY(angle_)));
    }
}
