#include <EasyDX/Game.hpp>
#include <EasyDx/GameWindow.hpp>
#include <memory>
#include "MainScene.hpp"
#include <EasyDX/LinkWithDirectX.hpp>

int main()
{
    dx::RunGame(std::make_unique<dx::GameWindow>(), std::make_shared<MainScene>());
}