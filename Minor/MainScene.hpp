#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/GameObject.hpp>
#include <EasyDx/Common.hpp>
#include <EasyDx/Light.hpp>
#include <EasyDx/Predefined.hpp>

class MainScene : public dx::Scene
{
public:
    MainScene(const dx::Game&, dx::Rc<void> args);
    void Update(const dx::UpdateArgs& updateArgs) override;

private:
    dx::Rc<dx::GameObject> sphere_, mirror_, wall_, floor_;
    dx::Light globalLight_;

    void BuildRoom(ID3D11Device& device, const dx::Predefined& predefined);
};