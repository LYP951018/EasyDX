#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Fwd.hpp>
#include "Pipeline.hpp"



class MainScene : public dx::Scene
{
public:
    MainScene(dx::Game&, dx::Rc<void> args);
    void Update(const dx::UpdateArgs&) override;
    ~MainScene() override;

private:
    wrl::ComPtr<ID3D11Buffer> instancingBuffer_;
    std::unique_ptr<Ball> ball_;
    std::array<dx::Light, 3> dirLights_;

    Pipeline MakePipeline(const dx::IndependentGraphics& independent, const dx::PredefinedResources& predefined);
    Box<Ball> MakeBall(const dx::IndependentGraphics& independent, const dx::PredefinedResources& predefined);
    void BuildLights();
};
