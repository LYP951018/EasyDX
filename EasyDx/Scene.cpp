#include "Scene.hpp"

namespace dx
{
    Camera& Scene::GetMainCamera() const noexcept
    {
        return *mainCamera_;
    }

    void Scene::SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept
    {
        mainCamera_ = std::move(mainCamera);
    }

    void Scene::Start()
    {
    }

    void Scene::Destroy() noexcept
    {
    }
}

