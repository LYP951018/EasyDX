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

        ID3D11Device& GetDevice3D() const;
        ID3D11DeviceContext& GetContext3D() const;
        IDXGIDevice& GetDxgiDevice() const;

        ID2D1Device& GetDevice2D() const;
        ID2D1Factory1& GetFactory2D() const;
        ID2D1DeviceContext& GetContext2D() const;

        IDWriteFactory1& GetDWriteFactory() const;

    private:
        friend Game& GetGame();

        Game();

        void InitializeDevices();

        wrl::ComPtr<ID3D11Device> device3D_;
        wrl::ComPtr<ID3D11DeviceContext> context3D_;
        wrl::ComPtr<ID2D1Factory1> d2dFactory_;
        wrl::ComPtr<ID2D1Device> device2D_;
        wrl::ComPtr<ID2D1DeviceContext> context2D_;
        wrl::ComPtr<IDXGIDevice> dxgiDevice_;

        wrl::ComPtr<IDWriteFactory1> dwFactory_;

        std::unique_ptr<GameWindow> mainWindow_;
    };
}