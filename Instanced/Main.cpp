#include "Pch.hpp"
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <EasyDx/LinkWithDirectX.hpp>
#include "Ball.hpp"

std::unique_ptr<dx::Scene> MakeScene(dx::Game & game)
{
    dx::Scene scene;
    auto& lights = scene.Lights();
    auto& camera = scene.MainCamera();
    auto& objects = scene.Objects();
    std::array<dx::DirectionalLight, 3> dirLights = {};
    dirLights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    dirLights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    dirLights[2].Direction = { 0.0f, -0.707f, -0.707f };
    for (auto& light : dirLights)
    {
        light.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        light.Enabled = true;
        lights.emplace_back(light);
    }
    objects.push_back(dx::MakeShared<Ball>(game.IndependentResources(), game.Predefined()));
    camera.UseDefaultMoveEvents(true);
    camera.Viewport() = dx::Rect{
        0.0f, 0.0f, 1.0f, 1.0f
    };
    camera.FarZ = 1000.0f;
    camera.NearZ = 1.0f;
    camera.Fov = DirectX::XM_PIDIV4;
    return dx::MakeUnique<dx::Scene>(std::move(scene));
}

int main()
{
    dx::IndependentGraphics independent;
    auto& loop = dx::EventLoop::GetInstanceInCurrentThread();
    auto window = dx::MakeUnique<dx::GameWindow>(loop);
    auto game = dx::MakeUnique<dx::Game>(std::move(independent), 30);
    game->Switcher().AddSceneCreator(0, MakeScene);
    dx::RunGame(*game, std::move(window), 0);
}


