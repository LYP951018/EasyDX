#include "pch.hpp"
#include "Game.hpp"
#include "GameWindow.hpp"
#include "Scene.hpp"
#include "EventLoop.hpp"
#include <stdexcept>
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <thread>

namespace dx
{
    void Game::Run()
    {
        const auto fps = fps_;
        const auto interval = std::chrono::microseconds{ 1000 / fps };
        auto& loop = EventLoop::GetInstanceInCurrentThread();
        TimePoint prev = Clock::now();
        auto& switcher = Switcher();
        auto& independent = IndependentResources();
        auto& context3D = independent.Context3D();
        auto& context2D = independent.Context2D();
        MainWindow().Show();
        loop.Run([&](WindowEventArgsPack e) {
            switcher.CheckAndSwitch();
            if (std::holds_alternative<IdleEventArgs>(e.Arg))
            {
                const auto now = Clock::now();
                const auto delta = now - prev;
                if (delta >= interval)
                {
                    UpdateArgs args{
                        delta,
                        context3D,
                        context2D
                    };
                    prev = now;
                    switcher.MainScene().Update(args);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
            else
            {
                UnpackMessage(std::move(e));
            }
        });
    }

    void Game::SetUp(std::unique_ptr<GameWindow> mainWindow)
    {
        mainWindow_ = std::move(mainWindow);
    }

    struct MessageDispatcher
    {
        Game& Game_;
        GameWindow* Window;

        void operator()(KeyEventArgs args) noexcept
        {
            switch (args.State)
            {
            case ElementState::Pressed:
                Game_.KeyDown(Window, args);
                break;
            case ElementState::Released:
                Game_.KeyUp(Window, args);
                break;
            }
        }

        void operator()(MouseEventArgs args) noexcept
        {
            switch (args.State)
            {
            case ElementState::Pressed:
                Game_.MouseDown(Window, args);
                break;
            case ElementState::Released:
                Game_.MouseUp(Window, args);
                break;
            }
        }

        void operator()(CursorMoved args) noexcept
        {
            Game_.MouseMove(Window, args);
        }

        void operator()(DpiChangedEventArgs args) noexcept
        {
            Window->PrepareForDpiChanging(args.NewDpiX, args.NewDpiY, args.NewRect);
            Game_.DpiChanged(Window, args);
        }

        void operator()(ResizeEventArgs args) noexcept
        {
            PrepareGraphicsForResizing(args.NewSize);
            Game_.WindowResize(Window, args);
        }

        template<typename T>
        void operator()(T)
        {
            assert(false);
        }

    private:
        void PrepareGraphicsForResizing(Size newSize)
        {
            auto& independent = Game_.IndependentResources();
            auto& context3D = independent.Context3D();
            auto& device3D = independent.Device3D();
            //auto& debug = independent.D3DDebug();
            auto& dependentGraphics = Game_.DependentResources();
            auto[newWidth, newHeight] = newSize;

            //first time resizement
            if (!dependentGraphics)
            {
                dependentGraphics.emplace(dx::DependentGraphics{
                    dx::SwapChain {
                        independent.Device3D(),
                        *Window,
                        dx::SwapChainOptions{}
                    },
                    dx::DepthStencil(independent.Device3D(),
                        newWidth, newHeight)
                    });
                return;
            }
            auto&[swapChain, depthStencil] = dependentGraphics.value();
            ID3D11RenderTargetView* nullViews[] = { nullptr };
            context3D.OMSetRenderTargets(gsl::narrow<UINT>(std::size(nullViews)), nullViews, {});
            depthStencil.Reset();
            swapChain.Reset();
            context3D.Flush();
            //debug.ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
            swapChain.Resize(device3D, newWidth, newHeight);
            //TODO: format should be cached.
            depthStencil = DepthStencil(device3D, newWidth, newHeight);
        }
    };

    void Game::UnpackMessage(WindowEventArgsPack event)
    {
        std::visit(MessageDispatcher{ *this, event.Window }, std::move(event.Arg));
    }

    void SceneSwitcher::ReallySwitchToScene([[maybe_unused]] Game& game, std::uint32_t index, std::shared_ptr<void> arg)
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

    IndependentGraphics::~IndependentGraphics()
    {}

    Game::Game(IndependentGraphics graphics, std::uint32_t fps)
        : fps_{fps},
        grahicsResources_{std::move(graphics)},
        predefined_{IndependentResources().Device3D()},
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

    IndependentGraphics::IndependentGraphics()
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
            gsl::narrow<UINT>(std::size(featureLevel)),
            D3D11_SDK_VERSION,
            device3D_.ReleaseAndGetAddressOf(),
            nullptr,
            context3D_.ReleaseAndGetAddressOf()
        ));

        TryHR(device3D_->QueryInterface(dxgiDevice_.ReleaseAndGetAddressOf()));
        TryHR(device3D_->QueryInterface(d3dDebug_.ReleaseAndGetAddressOf()));

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
}