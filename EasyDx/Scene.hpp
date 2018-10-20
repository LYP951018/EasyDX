#pragma once

#include "Events.hpp"
#include "Camera.hpp"
#include "Light.hpp"

namespace dx
{
    struct UpdateArgs;
    class PredefinedResources;

    // TODO: camera/light 都要加上 dirty 标记。
    class SceneBase
    {
      public:
        SceneBase(Game& game);
        Camera& MainCamera() { return mainCamera_; }
        const Camera& MainCamera() const { return mainCamera_; }
        std::vector<Light>& Lights() { return m_lights; }
        const std::vector<Light>& Lights() const { return m_lights; }

        virtual ~SceneBase();

        ID3D11Device& Device3D;
        const PredefinedResources& Predefined;

      private:
        friend class Game;
        virtual void Update(const UpdateArgs& args, const Game& game);
        virtual void Render(const Game& game);

        Camera mainCamera_;
        std::vector<Light> m_lights;
    };
} // namespace dx