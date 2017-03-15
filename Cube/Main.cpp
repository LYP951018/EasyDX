#include <EasyDX/Game.hpp>
#include <EasyDx/GameWindow.hpp>
#include <memory>
#include "MainScene.hpp"
#include <EasyDX/LinkWithDirectX.hpp>

int main()
{
    auto& game = dx::GetGame();
    game.AddScene(std::make_shared<MainScene>());
    dx::RunGame(game, std::make_unique<dx::GameWindow>(), 0);
}