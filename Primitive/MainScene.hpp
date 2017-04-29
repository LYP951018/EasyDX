#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Light.hpp>
#include <EasyDx/Objects.hpp>
#include <EasyDx/Events.hpp>
#include <d3d11.h>

class MainScene : public dx::Scene
{
protected:
    void Start(ID3D11Device&) override;
    void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) override;
    void Destroy() noexcept override;

private:
    dx::Object ball_;
    dx::PointLight light_;
    dx::EventHandle resized_;
    wrl::ComPtr<ID3D11Buffer> cbPerFrame_, cbPerObject_;
};