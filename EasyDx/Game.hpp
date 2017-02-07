#pragma once

#include "Common.hpp"
#include <memory>
#include <wrl/client.h>

namespace dx
{
    class GameWindow;

    class Game
    {
    public:
        void Run();
        Game& SetUp(std::unique_ptr<GameWindow> mainWindow);

        ID3D11Device& GetDevice() const;
        ID3D11DeviceContext& GetDeviceContext() const;

    private:
        friend Game& GetGame();

        Game();

        void InitializeDevice();

        wrl::ComPtr<ID3D11Device> device_;
        wrl::ComPtr<ID3D11DeviceContext> deviceContext_;
        std::unique_ptr<GameWindow> mainWindow_;
    };

    inline ID3D11Device& GetD3DDevice()
    {
        return GetGame().GetDevice();
    }

    inline ID3D11DeviceContext& GetDeviceContext()
    {
        return GetGame().GetDeviceContext();
    }
}