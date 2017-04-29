#include "Game.hpp"
#include "GameWindow.hpp"
#include "Common.hpp"
#include "Scene.hpp"
#include <chrono>
#include <stdexcept>
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite_1.h>

namespace dx
{
    void Game::Run()
    {
        MSG message = {};
        const auto fps = fps_;
        auto destroyMainScene = gsl::finally([this]() noexcept {
            scenes_[mainSceneIndex_]->Destroy();
            mainSceneIndex_ = InvalidSceneIndex;
        });
        mainWindow_->Show();
        auto& context3D = GetContext3D();
        auto& context2D = GetContext2D();
        //FIXME: 应该确保 WM_SIZE 在第一次 Window->Draw 之前。
        auto prevTime = std::chrono::steady_clock::now();
        using namespace std::chrono_literals;
        auto totalTime = 0ms;
        while (true)
        {
            if (PeekMessage(&message, {}, {}, {}, PM_REMOVE) != 0)
            {
                if (message.message == WM_QUIT)
                {
                    break;
                }
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                const auto nowTime = std::chrono::steady_clock::now();
                const auto delta = (std::max)(std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - prevTime), 0ms);
                if (delta >= std::chrono::milliseconds(1000 / fps))
                {
                    totalTime += delta;
                    UpdateArgs updateArgs{ delta, totalTime, context3D, context2D };
                    prevTime = nowTime;
                    auto& mainScene = *GetMainScene();
                    mainScene.Update(updateArgs);
                    mainScene.Render(context3D, context2D);
                    GetMainWindow()->Present();
                }
            }
        }
    }

    void Game::SetUp(std::unique_ptr<GameWindow> mainWindow)
    {
        mainWindow_ = std::move(mainWindow);
    }

    void Game::AddScene(std::shared_ptr<Scene> scene)
    {
        scenes_.push_back(std::move(scene));
    }

    void Game::SwitchToScene(std::uint32_t index)
    {
        if (index >= scenes_.size())
            throw std::logic_error{ "Invalid scene index!" };
        if (mainSceneIndex_ != InvalidSceneIndex)
        {
            scenes_[mainSceneIndex_]->Destroy();
        }
        mainSceneIndex_ = index;
        auto mainScene = scenes_[index].get();
        mainScene->Start(GetDevice3D());
    }

    std::shared_ptr<Scene> Game::GetMainScene() const
    {
        return scenes_[mainSceneIndex_];
    }

    GameWindow* Game::GetMainWindow() const
    {
        return mainWindow_.get();
    }

    ID3D11Device& Game::GetDevice3D() const
    {
        return Ref(device3D_);
    }

    ID3D11DeviceContext& Game::GetContext3D() const
    {
        return Ref(context3D_);
    }

    IDXGIDevice& Game::GetDxgiDevice() const
    {
        return Ref(dxgiDevice_);
    }

    ID2D1Device& Game::GetDevice2D() const
    {
        return Ref(device2D_);
    }

    ID2D1Factory1& Game::GetFactory2D() const
    {
        return Ref(d2dFactory_);
    }

    ID2D1DeviceContext& Game::GetContext2D() const
    {
        return Ref(context2D_);
    }

    IDWriteFactory1& Game::GetDWriteFactory() const
    {
        return Ref(dwFactory_);
    }

    Game::Game()
        : mainSceneIndex_{InvalidSceneIndex},
        fps_{60}
    {
        TryHR(::CoInitialize(nullptr));
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

    Game& GetGame()
    {
        static Game game;
        return game;
    }

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex)
    {
        //Step 1: setup the main window.
        game.SetUp(std::move(mainWindow));
        //Step 2: switch to the main scene, in which Scene::Start will be called, which may depend on MainWindow.
        //There always be event registration in Scene::Start, so we must ensure GameWindow::Show be executed after
        //Start.
        game.SwitchToScene(mainSceneIndex);
        //Step 3: run the game.
        game.Run();
    }
}