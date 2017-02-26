#include <EasyDx/Game.hpp>
#include <memory>
#include "MainWindow.hpp"
#include <EasyDx/LinkWithDirectX.hpp>

int main()
{
    dx::GetGame()
        .SetUp(std::make_unique<MainWindow>())
        .Run();
}