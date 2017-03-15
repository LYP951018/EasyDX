#include "MainScene.hpp"
#include <EasyDx/Game.hpp>
#include <EasyDx/GameWindow.hpp>
#include <EasyDx/LinkWithDirectX.hpp>
#include <memory>


int main()
{
    auto& game = dx::GetGame();
    game.AddScene(std::make_shared<MainScene>());
    dx::RunGame(game, std::make_unique<dx::GameWindow>(), 0);
}