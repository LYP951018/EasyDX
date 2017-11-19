#include "pch.hpp"
#include "Game.hpp"
#include "GameWindow.hpp"
#include "Scene.hpp"
#include "BasicVS.hpp"
#include "BasicPS.hpp"
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
        auto& switcher = Switcher();
        auto destroyMainScene = gsl::finally([&]() noexcept {
            switcher.Reset();
        });
        mainWindow_->Show();
        auto& resources = Resources();
        auto& context3D = resources.Context3D();
        auto& context2D = resources.Context2D();
        //FIXME: 应该确保 WM_SIZE 在第一次 Window->Draw 之前。
        auto prevTime = std::chrono::steady_clock::now();
        using namespace std::chrono_literals;
        auto totalTime = 0ms;
        while (true)
        {
            switcher.CheckAndSwitch();
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
                    auto& mainScene = switcher.MainScene();
                    mainScene.Update(updateArgs);
                    MainWindow().Present();
                }
            }
        }
    }

    void Game::SetUp(std::unique_ptr<GameWindow> mainWindow)
    {
        mainWindow_ = std::move(mainWindow);
    }

    void SceneSwitcher::ReallySwitchToScene(Game& game, std::uint32_t index, std::shared_ptr<void> arg)
    {
        const auto it = sceneCreators_.find(index);
        if (it != sceneCreators_.end())
        {
            const auto creator = std::move(it->second);
            sceneCreators_.erase(it);
            mainScene_ = creator(game_, std::move(arg));
        }
        else
            throw std::logic_error{ "Invalid index!" };
    }

    void SceneSwitcher::CheckAndSwitch()
    {
        if (nextSceneIndex_)
        {
            ReallySwitchToScene(game_, nextSceneIndex_.value(), std::move(nextSceneArg_));
            nextSceneIndex_ = std::nullopt;
        }
    }

    void SceneSwitcher::Reset()
    {
        mainScene_.reset();
        nextSceneIndex_ = std::nullopt;
    }

    SceneSwitcher::SceneSwitcher(Game& game)
        : game_{game}
    {
    }

    void SceneSwitcher::AddSceneCreator(std::uint32_t index, SceneCreator creator)
    {
        sceneCreators_.insert(std::make_pair(index, std::move(creator)));
    }

    void SceneSwitcher::WantToSwitchSceneTo(std::uint32_t index, std::shared_ptr<void> arg)
    {
        nextSceneIndex_ = index;
        nextSceneArg_ = std::move(arg);
    }

    Game::Game(std::uint32_t fps)
        : fps_{fps},
        predefined_{Resources().Device3D()},
        sceneSwitcher_{*this}
    {
        TryHR(::CoInitialize(nullptr));
    }

    Game::~Game()
    {
    }

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex, std::shared_ptr<void> arg)
    {
        //Step 1: setup the main window.
        game.SetUp(std::move(mainWindow));
        //Step 2: switch to the main scene, in which Scene::Start will be called, which may depend on MainWindow.
        //There always be event registration in Scene::Start, so we must ensure GameWindow::Show be executed after
        //Start.
        auto& sceneSwitcher = game.Switcher();
        sceneSwitcher.ReallySwitchToScene(game, mainSceneIndex, std::move(arg));
        //Step 3: run the game.
        game.Run();
    }

    GraphicsResources::GraphicsResources()
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

        TryHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, fanctory2D_.ReleaseAndGetAddressOf()));
        TryHR(fanctory2D_->CreateDevice(dxgiDevice_.Get(), device2D_.ReleaseAndGetAddressOf()));
        TryHR(device2D_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, context2D_.ReleaseAndGetAddressOf()));

        TryHR(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory1),
            reinterpret_cast<IUnknown**>(dwFactory_.ReleaseAndGetAddressOf())));
    }

    GraphicsResources::~GraphicsResources()
    {
    }
}