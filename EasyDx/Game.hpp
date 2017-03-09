#pragma once

#include "Common.hpp"
#include <memory>
#include <unordered_map>
#include <string>

namespace dx
{
    class GameWindow;
    class Scene;

    class Game
    {
    public:
        void Run();
        void SetUp(std::unique_ptr<GameWindow> mainWindow);
        void AddScene(std::string name, std::shared_ptr<Scene> scene);
        std::shared_ptr<Scene> GetScene(const std::string& name) const;
        void SetMainScene(const std::string& name);
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
        std::unordered_map<std::string, std::shared_ptr<Scene>> scenes_;

        //TODO: raw pointer?
        std::shared_ptr<Scene> mainScene_;
    };


    void RunGame(std::unique_ptr<GameWindow> mainWindow, std::shared_ptr<Scene> mainScene);
}