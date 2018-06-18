#pragma once

#include "Events.hpp"
#include "Camera.hpp"
#include "Light.hpp"

namespace dx
{
    class ObjectBase;
    struct UpdateArgs;

    class Scene final : Noncopyable
    {
    public:
        template<typename...Args, typename F>
        void RegisterEvent(Events<Args...>& event, F&& callback)
        {
            eventHandles_.push_back(event.Add(std::forward<F>(callback)));
        }

        Camera& MainCamera()
        {
            return mainCamera_;
        }

        const Camera& MainCamera() const
        {
            return mainCamera_;
        }

        std::vector<Light>& Lights()
        {
            return m_lights;
        }

        const std::vector<Light>& Lights() const
        {
            return m_lights;
        }

        using ObjectsContainer = std::vector<std::shared_ptr<ObjectBase>>;

        ObjectsContainer& Objects()
        {
            return m_objects;
        }

        const ObjectsContainer& Objects() const
        {
            return m_objects;
        }

    private:
        friend class Game;
        void Update(const UpdateArgs& args, const Game& game);
        void Render(const Game& game);

        Camera mainCamera_;
        std::vector<Light> m_lights;
        std::vector<std::unique_ptr<IEventHandle>> eventHandles_;
        std::vector<std::shared_ptr<ObjectBase>> m_objects;
    };
}