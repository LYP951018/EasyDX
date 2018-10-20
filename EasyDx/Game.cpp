#include "pch.hpp"
#include "Game.hpp"
#include "GameWindow.hpp"
#include "Scene.hpp"
#include "EventLoop.hpp"
#include "InputSystem.hpp"
#include "GraphicsDevices.hpp"
#include "DependentGraphics.hpp"
#include <stdexcept>
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <thread>
#include <DirectXColors.h>

namespace dx
{
    void Game::Run()
    {
        const auto fps = fps_;
        const auto interval = std::chrono::milliseconds{ 1000 / fps };
        auto& loop = EventLoop::GetInstanceInCurrentThread();
        TimePoint prev = Clock::now();
        auto& switcher = Switcher();
        auto& independent = IndependentResources();
        auto& context3D = independent.Context3D();
        //auto& context2D = independent.Context2D();
        MainWindow().Show();
        loop.Run([&](WindowEventArgsPack e) {
            //wait until the first resize event reach.
            if (std::holds_alternative<ResizeEventArgs>(e.Arg))
            {
                bool firstResize = !m_dependentGraphics.has_value();
                auto& resizeArg = std::get<ResizeEventArgs>(e.Arg);
                e.Window->OnResize(resizeArg.NewSize);
                PrepareGraphicsForResizing(e.Window, resizeArg.NewSize);
                if (firstResize)
                {
                    m_inputSystem->OnInitialized();
                }
            }
            else if (!m_dependentGraphics)
            {
                return;
            }

            //not resize event, and initialized.
            switcher.CheckAndSwitch();
            if (std::holds_alternative<IdleEventArgs>(e.Arg))
            {
                const auto now = Clock::now();
                const auto delta = now - prev;
                if (delta >= interval)
                {
                    UpdateArgs args{ delta };
                    prev = now;
                    m_inputSystem->OnFrameStart();
                    auto& scene = switcher.MainScene();
                    auto& camera = scene.MainCamera();
                    camera.PrepareForRendering(context3D, *this);
                    scene.Update(args, *this);
                    camera.Update(args, *this);
                    auto& dependent = m_dependentGraphics.value();
                    //TODO: clean up, move these to the Camera class
                    /*dependent.DepthStencil_.ClearBoth(context3D);
                    dependent.SwapChain_.Front().Clear(context3D, DirectX::Colors::Black);*/
                    //Ugh
                    dependent.Bind(context3D);
                    scene.Render(*this);
                    dependent.SwapChain_.Present();
                    m_inputSystem->OnFrameDone();
                }
                else
                {
                    Sleep(1);
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
                Game_.GetInputSystem().OnKeyDown(args.Key);
                break;
            case ElementState::Released:
                Game_.GetInputSystem().OnKeyUp(args.Key);
                break;
            }
        }

        void operator()(MouseEventArgs) noexcept
        {
            //ignore WM_*BUTTON{DOWN, UP}, handle KEY messages only.
        }

        void operator()(CursorMoved) noexcept
        {
            //Game_.GetInputSystem().OnMouseMove(args.Position);
        }

        void operator()(DpiChangedEventArgs args) noexcept
        {
            Window->PrepareForDpiChanging(args.NewDpiX, args.NewDpiY, args.NewRect);
            Game_.DpiChanged(Window, args);
        }

        void operator()(ResizeEventArgs args) noexcept
        {
            //do we need this?
            Game_.WindowResize(Window, args);
        }

        template<typename T>
        void operator()(T)
        {
            assert(false);
        }
    };

    void Game::UnpackMessage(WindowEventArgsPack event)
    {
        std::visit(MessageDispatcher{ *this, event.Window }, std::move(event.Arg));
    }

    void Game::PrepareGraphicsForResizing(GameWindow* window, Size newSize)
    {
        auto& independent = IndependentResources();
        auto& context3D = independent.Context3D();
        auto& device3D = independent.Device3D();
        //auto& debug = independent.D3DDebug();
        auto& dependentGraphics = m_dependentGraphics;
        auto[newWidth, newHeight] = newSize;
        auto& camera = Switcher().MainScene().MainCamera();
        camera.OnResize(newSize);
        //first time resizement
        if (!dependentGraphics)
        {
            dependentGraphics.emplace(dx::DependentGraphics{
                dx::SwapChain{
                    independent.Device3D(),
                    *window,
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

    void SceneSwitcher::ReallySwitchToScene([[maybe_unused]] Game& game, std::uint32_t index)
    {
        const auto it = sceneCreators_.find(index);
        if (it != sceneCreators_.end())
        {
            const auto creator = std::move(it->second);
            sceneCreators_.erase(it);
            mainScene_ = creator(game_);
        }
        else
            throw std::logic_error{ "Invalid index!" };
    }

    void SceneSwitcher::CheckAndSwitch()
    {
        if (nextSceneIndex_)
        {
            ReallySwitchToScene(game_, nextSceneIndex_.value());
            nextSceneIndex_ = std::nullopt;
        }
    }

    void SceneSwitcher::Reset()
    {
        mainScene_.reset();
        nextSceneIndex_ = std::nullopt;
    }

    SceneSwitcher::SceneSwitcher(Game& game)
        : game_{ game }
    {
    }

    void SceneSwitcher::AddSceneCreator(std::uint32_t index, SceneCreator creator)
    {
        sceneCreators_.insert(std::make_pair(index, std::move(creator)));
    }

    void SceneSwitcher::WantToSwitchSceneTo(std::uint32_t index)
    {
        nextSceneIndex_ = index;
    }

    IndependentGraphics::~IndependentGraphics()
    {}

    Game::Game(IndependentGraphics graphics, std::uint32_t fps)
        : fps_{ fps },
        grahicsResources_{ std::move(graphics) },
        predefined_{ IndependentResources().Device3D() },
        sceneSwitcher_{ *this },
        m_inputSystem{ MakeUnique<InputSystem>() }
    {
        TryHR(::CoInitialize(nullptr));
    }

    Game::~Game()
    {
    }

    void RunGame(Game& game, std::unique_ptr<GameWindow> mainWindow, std::uint32_t mainSceneIndex)
    {
        //Step 1: setup the main window.
        game.SetUp(std::move(mainWindow));
        //Step 2: switch to the main scene, in which Scene::Start will be called, which may depend on MainWindow.
        //There always be event registration in Scene::Start, so we must ensure GameWindow::Show be executed after
        //Start.
        auto& sceneSwitcher = game.Switcher();
        sceneSwitcher.ReallySwitchToScene(game, mainSceneIndex);
        //Step 3: run the game.
        game.Run();
    }

    IndependentGraphics::IndependentGraphics()
    {
        std::tie(device3D_, context3D_) = MakeDevice3D();
        TryHR(device3D_->QueryInterface(dxgiDevice_.ReleaseAndGetAddressOf()));
#if _DEBUG
        TryHR(device3D_->QueryInterface(d3dDebug_.ReleaseAndGetAddressOf()));
#else
        device3D_->QueryInterface(d3dDebug_.ReleaseAndGetAddressOf());
#endif

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