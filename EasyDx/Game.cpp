#include "Game.hpp"
#include "GameWindow.hpp"
#include "Common.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite_1.h>

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

    ID3D11Device& Game::GetDevice3D() const
    {
        return *device3D_.Get();
    }

    ID3D11DeviceContext& Game::GetContext3D() const
    {
        return *context3D_.Get();
    }

    IDXGIDevice& Game::GetDxgiDevice() const
    {
        return *dxgiDevice_.Get();
    }

    ID2D1Device& Game::GetDevice2D() const
    {
        return *device2D_.Get();
    }

    ID2D1Factory1 & Game::GetFactory2D() const
    {
        return *d2dFactory_.Get();
    }

    ID2D1DeviceContext& Game::GetContext2D() const
    {
        return *context2D_.Get();
    }

    IDWriteFactory1& Game::GetDWriteFactory() const
    {
        return *dwFactory_.Get();
    }

    Game::Game()
    {
        InitializeDevices();
    }

    void Game::InitializeDevices()
    {
        UINT creationFlags = {};
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        creationFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        const D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };

        TryHR(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevel,
            std::size(featureLevel),
            D3D11_SDK_VERSION,
            device3D_.ReleaseAndGetAddressOf(),
            nullptr,
            context3D_.ReleaseAndGetAddressOf()
        ));

        TryHR(device3D_->QueryInterface(dxgiDevice_.ReleaseAndGetAddressOf()));

        D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        TryHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2dFactory_.ReleaseAndGetAddressOf()));
        TryHR(d2dFactory_->CreateDevice(dxgiDevice_.Get(), device2D_.ReleaseAndGetAddressOf()));
        TryHR(device2D_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, context2D_.ReleaseAndGetAddressOf()));

        TryHR(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory1),
            reinterpret_cast<IUnknown**>(dwFactory_.ReleaseAndGetAddressOf())));
    }

    Game & GetGame()
    {
        static Game game;
        return game;
    }
}