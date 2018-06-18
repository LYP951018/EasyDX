#include <EasyDx/One.hpp>
#include <array>
#include "MainScene.hpp"

using namespace dx;




MainScene::MainScene(dx::Game& game, dx::Rc<void> args)
    : Scene{game},
    ball_{MakeBall(game.IndependentResources(), game.Predefined())}
{
}



void MainScene::Update(const dx::UpdateArgs& args)
{
    auto& game = GetGame();
    auto& dependentOpt = game.DependentResources();
    if (!dependentOpt) return;
    auto& dependent = dependentOpt.value();
    auto& context3D = args.Context3D;
    auto& camera = GetMainCamera();
    const auto lights = gsl::make_span(dirLights_);
    auto& pipeline = ball_->Pipeline_;
    dependent.Bind(context3D);
    context3D.RSSetViewports(1, &camera.Viewport());
    Bind(context3D, pipeline);
    const auto drawContext = dx::BasicDrawContext{
        context3D, camera, lights
    };
    SetupBasicLighting(drawContext, pipeline.PS, ball_->Material);
    DrawAllIndexed(context3D, ball_->Pipeline_);
    dependent.SwapChain_.Present();
}

void MainScene::BuildLights()
{
    
    std::copy(dirLights.begin(), dirLights.end(), dirLights_.begin());
}
