#include <EasyDX/Game.hpp>
#include <memory>
#include "MainWindow.hpp"
#include <EasyDX/LinkWithDirectX.hpp>

int main()
{
    dx::GetGame()
        .SetUp(std::make_unique<MainWindow>())
        .Run();
}