#include <Game.hpp>
#include <memory>
#include "MainWindow.hpp"

int main()
{
    dx::GetGame()
        .SetUp(std::make_unique<MainWindow>())
        .Run();
}