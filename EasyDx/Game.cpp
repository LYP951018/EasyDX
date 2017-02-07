#include "Game.hpp"
#include "GameWindow.hpp"
#include "Common.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>

namespace dx
{
    void Game::Run()
    {
        mainWindow_->Show();
        BOOL ret;
        MSG message;
        while ((ret = GetMessage(&message, nullptr, 0, 0)) != 0)
        {
            if (ret == -1)
            {
                //TODO: throw
                break;
            }
            else
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
    }

    Game& Game::SetUp(std::unique_ptr<GameWindow> mainWindow)
    {
        mainWindow_ = std::move(mainWindow);
        return *this;
    }

    ID3D11Device& Game::GetDevice() const
    {
        return *device_.Get();
    }

    ID3D11DeviceContext& Game::GetDeviceContext() const
    {
        return *deviceContext_.Get();
    }

    Game::Game()
    {
        InitializeDevice();
    }

    void Game::InitializeDevice()
    {
        UINT creationFlags = {};
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        const D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };

        TryHR(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevel,
            std::size(featureLevel),
            D3D11_SDK_VERSION,
            device_.ReleaseAndGetAddressOf(),
            nullptr,
            deviceContext_.ReleaseAndGetAddressOf()
        ));
    }

    Game & GetGame()
    {
        static Game game;
        return game;
    }
}