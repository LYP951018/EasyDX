#include <Game.hpp>
#include <memory>
#include "MainWindow.hpp"
#include <LinkWithDirectX.hpp>

int main()
{
    dx::GetGame()
        .SetUp(std::make_unique<MainWindow>())
        .Run();
}