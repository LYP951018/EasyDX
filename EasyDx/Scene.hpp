#pragma once

#include "DXDef.hpp"
#include "Events.hpp"
#include <optional>
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
        Scene(Game& game);
        virtual ~Scene();

        Camera& GetMainCamera() const noexcept;
        Game& GetGame() const { return game_; }

    protected:
        virtual void Update(const UpdateArgs& updateArgs);

        template<typename T, typename F>
        void RegisterEvent(Event<T>& event, F&& callback)
        {
            eventHandles_.push_back(event.Add(std::forward<F>(callback)));
        }

        void AddCameraMovement();

    private:
        void AddBasicEvents();
        
        Game& game_;
        //XXX
        std::optional<dx::Point> oldPoint_;
        //TODO: remove unique_ptr.
        std::unique_ptr<Camera> mainCamera_;
        std::vector<std::unique_ptr<IEventHandle>> eventHandles_;
    };


    template<typename SceneT>
    struct BasicSceneCreator
    {
        static_assert(std::is_base_of_v<Scene, SceneT>, "SceneT should derive from dx::Scene");

        std::unique_ptr<Scene> operator()(Game& game, std::shared_ptr<void> arg)
        {
            return std::make_unique<SceneT>(game, std::move(arg));
        }
    };
}