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

    Scene::Scene(Game& game)
        : game_{ game },
        mainCamera_{std::make_unique<Camera>()}
    {
        AddBasicEvents();
    }

    Camera& Scene::GetMainCamera() const noexcept
    {
        return *mainCamera_;
    }

    void Scene::Update(const UpdateArgs&)
    {
    }

    void Scene::AddBasicEvents()
    {
        auto& camera = GetMainCamera();
        RegisterEvent(GetGame().WindowResize, [&](GameWindow*, dx::ResizeEventArgs& e) {
            camera.SetProjection(DirectX::XM_PIDIV4, e.NewSize.GetAspectRatio(), 0.01f, 1000.f);
            camera.Viewport() = {
                0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
                0.f, 1.f
            };
        });
    }

    void Scene::AddCameraMovement()
    {
        RegisterEvent(game_.MouseDown, [&](GameWindow*, dx::MouseEventArgs& args) {
            if (args.KeyStates_.Left())
            {
                oldPoint_ = args.Position;
            }
        });

        RegisterEvent(game_.MouseMove, [&](GameWindow*, dx::CursorMoved& args) {
            if (oldPoint_ && args.KeyStates_.Left())
            {
                auto& oldPos = oldPoint_.value();
                const auto newPos = args.Position;
                const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(newPos.X - oldPos.X));
                const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(newPos.Y - oldPos.Y));
                auto& camera = GetMainCamera();
                camera.RotateY(dx);
                camera.RotateX(dy);
                oldPos = newPos;
            }
        });

        RegisterEvent(game_.KeyDown, [&](GameWindow*, dx::KeyEventArgs& args) {
            auto& camera = GetMainCamera();
            switch (args.Key)
            {
            case VK_UP:
                camera.Walk(1.0f);
                break;
            case VK_DOWN:
                camera.Walk(-1.0f);
                break;
            case VK_LEFT:
                camera.Strafe(-1.0f);
                break;
            case VK_RIGHT:
                camera.Strafe(1.0f);
                break;
            default:
                break;
            }
        });

        RegisterEvent(game_.MouseUp, [&](GameWindow*, dx::MouseEventArgs&) {
            oldPoint_ = std::nullopt;
        });
    }
}

