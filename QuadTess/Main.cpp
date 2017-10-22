#include <EasyDx/One.hpp>
#include <EasyDx/LinkWithDirectX.hpp>
#include "MainScene.hpp"

int main()
{
    auto game = std::make_unique<dx::Game>();
    game->AddSceneCreator(0, dx::BasicSceneCreator<MainScene>{});
    dx::RunGame(*game, std::make_unique<dx::GameWindow>(*game), 0, {});
}