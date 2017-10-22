#pragma once

#include "DXDef.hpp"
#include "Events.hpp"
#include <chrono>

namespace dx
{
    //poor man's scene
    class Camera;

    struct UpdateArgs
    {
        std::chrono::milliseconds DeltaTime;
        std::chrono::milliseconds TotalTime;
        ID3D11DeviceContext& Context3D;
        ID2D1DeviceContext& Context2D;
    };

    class Scene
    {
        friend class GameWindow;
        friend class Game;

    public:
        Scene(const Game& game);
        virtual ~Scene();

        Camera& GetMainCamera() const noexcept;
        void SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept;
        const Game& GetGame() const { return game_; }

    protected:
        virtual void Update(const UpdateArgs& updateArgs);

    private:
        dx::EventHandle<dx::WindowResizeEvent> AddResize();
        const Game& game_;
        std::unique_ptr<Camera> mainCamera_;
        dx::EventHandle<dx::WindowResizeEvent> resize_;
    };


    template<typename SceneT>
    struct BasicSceneCreator
    {
        static_assert(std::is_base_of_v<Scene, SceneT>, "SceneT should derive from dx::Scene");

        std::unique_ptr<Scene> operator()(const Game& game, std::shared_ptr<void> arg)
        {
            return std::make_unique<SceneT>(game, std::move(arg));
        }
    };
}