#include "Pch.hpp"
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <EasyDx/LinkWithDirectX.hpp>
#include "MainScene.hpp"

int main()
{
    dx::IndependentGraphics independent;
    auto& loop = dx::EventLoop::GetInstanceInCurrentThread();
    auto window = dx::MakeUnique<dx::GameWindow>(loop);
    auto game = dx::MakeUnique<dx::Game>(std::move(independent), 30);
    game->Switcher().AddSceneCreator(
        0, [](dx::Game& game) { return dx::MakeUnique<MainScene>(game); });
    dx::RunGame(*game, std::move(window), 0);
}


