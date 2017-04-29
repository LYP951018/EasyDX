#include <EasyDX/Game.hpp>
#include <EasyDx/GameWindow.hpp>
#include <memory>
#include <EasyDX/LinkWithDirectX.hpp>
#include "MainScene.hpp"


int main()
{
    auto& game = dx::GetGame();
    game.AddScene(std::make_shared<MainScene>());
    dx::RunGame(game, std::make_unique<dx::GameWindow>(), 0);
}