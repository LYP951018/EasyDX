#include "Scene.hpp"
#include "Camera.hpp"

namespace dx
{
    Scene::~Scene()
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

    void Scene::Start(ID3D11Device&)
    {
    }

    void Scene::Destroy() noexcept
    {
    }
}

