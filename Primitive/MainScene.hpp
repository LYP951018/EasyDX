#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Light.hpp>
#include <EasyDx/Objects.hpp>
#include <EasyDx/Events.hpp>
#include <d3d11.h>

class MainScene : public dx::Scene
{
public:
    MainScene(const dx::Game&, std::shared_ptr<void>);

protected:
    void Update(const dx::UpdateArgs& args) override;
    void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) override;

private:
    dx::EventHandle<dx::WindowResizeEvent> AddResize();
    dx::Object ball_;
    dx::PointLight light_;
    dx::EventHandle<dx::WindowResizeEvent> resized_;
    wrl::ComPtr<ID3D11Buffer> cbPerFrame_, cbPerObject_;
};