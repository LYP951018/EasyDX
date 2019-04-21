#include "pch.hpp"
#include <EasyDx/LinkWithDirectX.hpp>
#include <d2d1_1.h>
#include <dwrite_1.h>
#include "MainScene.hpp"

using namespace DirectX;

int main()
{
    auto& loop = dx::EventLoop::GetInstanceInCurrentThread();
    auto window = dx::MakeUnique<dx::GameWindow>(loop);
    dx::Game game{
        std::make_unique<dx::GlobalGraphicsContext>(
            dx::DefaultSwapChainDescFromWindowHandle(window->NativeHandle())),
        30};
    auto& device3D = game.GlobalGraphics().Device3D();
    dx::Shaders::Setup();
    dx::Shaders::LoadDefaultShaders(device3D);
    dx::InputLayoutAllocator::Setup();
    dx::InputLayoutAllocator::LoadDefaultInputLayouts(device3D);
    dx::PredefinedResources::Setup(device3D);
    game.Switcher().AddSceneCreator(
        0, [](dx::Game& game) { return dx::MakeUnique<MainScene>(game); });
    dx::RunGame(game, std::move(window), 0);
}