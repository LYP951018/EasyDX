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
        auto& gfxContext = GlobalGraphics();
        auto& context3D = gfxContext.Context3D();
		bool resized = false;
        //auto& context2D = independent.Context2D();
        MainWindow().Show();
        m_inputSystem->OnInitialized();
        loop.Run([&](WindowEventArgsPack e) {
            //wait until the first resize event reach.
            if (std::holds_alternative<ResizeEventArgs>(e.Arg))
            {
                auto& resizeArg = std::get<ResizeEventArgs>(e.Arg);
                e.Window->OnResize(resizeArg.NewSize);
                PrepareGraphicsForResizing(e.Window, resizeArg.NewSize);
				resized = true;
            }
			if (!resized) return;
            //not a resize event, and initialized.
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
                    camera.Update(args, *this);
                    scene.Update(args, *this);
                    scene.Render(context3D, gfxContext, *this);
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

    void Game::PrepareGraphicsForResizing(GameWindow*, Size newSize)
    {
        auto& globalGraphics = GlobalGraphics();
        auto& camera = Switcher().MainScene().MainCamera();
        camera.OnResize(newSize);
        globalGraphics.OnResize(newSize);
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

    Game::Game(std::unique_ptr<GlobalGraphicsContext> globalGraphics, 
        std::uint32_t fps)
        : fps_{fps}, m_globalGraphics{std::move(globalGraphics)},
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


}