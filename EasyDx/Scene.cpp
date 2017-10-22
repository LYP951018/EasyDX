#include "pch.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Game.hpp"
#include "GameWindow.hpp"

namespace dx
{
    Scene::~Scene()
    {
    }

    Scene::Scene(const Game& game)
        : game_{ game },
        mainCamera_{std::make_unique<Camera>()},
        resize_{AddResize()}
    {
    }

    Camera& Scene::GetMainCamera() const noexcept
    {
        return *mainCamera_;
    }

    void Scene::SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept
    {
        mainCamera_ = std::move(mainCamera);
    }

    void Scene::Update(const UpdateArgs&)
    {
    }

    auto Scene::AddResize() -> dx::EventHandle<dx::WindowResizeEvent>
    {
        auto& camera = GetMainCamera();
        auto mainWindow = GetGame().GetMainWindow();
        return mainWindow->WindowResize.Add([&](dx::ResizeEventArgs& e) {
            camera.SetProjection(DirectX::XM_PIDIV4, e.NewSize.GetAspectRatio(), 0.01f, 1000.f);
            camera.MainViewport = {
                0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
                0.f, 1.f
            };
        });
    }
}

