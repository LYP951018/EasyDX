#include "pch.hpp"
#include "Scene.hpp"
#include "Camera.hpp"

namespace dx
{
    Scene::~Scene()
    {
    }

    Scene::Scene()
        : mainCamera_{std::make_unique<Camera>()}
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
}

