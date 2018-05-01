#include <EasyDx/One.hpp>
#include <EasyDx/LinkWithDirectX.hpp>
#include "MainScene.hpp"

int main()
{
    dx::IndependentGraphics independent;
    auto window = dx::MakeUnique<dx::GameWindow>(dx::GameWindow())
    auto game = dx::MakeUnique<dx::Game>(30);
    game->Switcher().AddSceneCreator(0, dx::BasicSceneCreator<MainScene>{});
    dx::RunGame(*game, std::make_unique<dx::GameWindow>(*game), 0, {});
}