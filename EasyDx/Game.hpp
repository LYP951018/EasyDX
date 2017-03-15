#pragma once

#include "Common.hpp"
#include <memory>
#include <vector>
#include <cstdint>
#include <climits>

namespace dx
{
    class GameWindow;
    class Scene;

    class Game
    {
        friend void RunGame(Game&, std::unique_ptr<GameWindow>, std::uint32_t);

    public:
        void AddScene(std::shared_ptr<Scene> scene);
        void SwitchToScene(std::uint32_t index);
        std::shared_ptr<Scene> GetMainScene() const;
        GameWindow* GetMainWindow() const;

        ID3D11Device& GetDevice3D() const;
        ID3D11DeviceContext& GetContext3D() const;
        IDXGIDevice& GetDxgiDevice() const;

        ID2D1Device& GetDevice2D() const;
        ID2D1Factory1& GetFactory2D() const;
        ID2D1DeviceContext& GetContext2D() const;

        IDWriteFactory1& GetDWriteFactory() const;

    private:
        friend Game& GetGame();

        static constexpr std::uint32_t InvalidSceneIndex = UINT32_MAX;

        Game();

        void InitializeDevices();
        void Run();
        void SetUp(std::unique_ptr<GameWindow> mainWindow);

        wrl::ComPtr<ID3D11Device> device3D_;
        wrl::ComPtr<ID3D11DeviceContext> context3D_;
        wrl::ComPtr<ID2D1Factory1> d2dFactory_;
        wrl::ComPtr<ID2D1Device> device2D_;
        wrl::ComPtr<ID2D1DeviceContext> context2D_;
        wrl::ComPtr<IDXGIDevice> dxgiDevice_;

        wrl::ComPtr<IDWriteFactory1> dwFactory_;

        std::unique_ptr<GameWindow> mainWindow_;
        std::vector<std::shared_ptr<Scene>> scenes_;

        std::uint32_t mainSceneIndex_;
    };

    Game& GetGame();
    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex);
}