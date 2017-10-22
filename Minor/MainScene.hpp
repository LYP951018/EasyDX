#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Common.hpp>
#include <EasyDx/Light.hpp>
#include <EasyDx/Predefined.hpp>
#include <array>

class MainScene : public dx::Scene
{
public:
    MainScene(const dx::Game&, dx::Rc<void> args);
    void Update(const dx::UpdateArgs& updateArgs) override;
    ~MainScene() override;

private:
    dx::BasicObject sphere_, mirror_, wall_, floor_, reflectedSphere_, sphereShadow_;
    std::array<dx::Light, 3> dirLights_;
    dx::WindowResizeEventHandle resized_;

    void BuildRoom(ID3D11Device& device, const dx::Predefined& predefined);
    void BuildLights();
    dx::EventHandle<dx::WindowResizeEvent> AddResize();
};