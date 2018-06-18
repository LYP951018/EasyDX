#include "pch.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "ObjectBase.hpp"
#include "Game.hpp"

namespace dx
{
    void Scene::Update(const UpdateArgs& args, const Game& game)
    {
        mainCamera_.Update(args, game);
        for (const auto& object : m_objects)
        {
            object->Update(args, game);
        }
    }

    void Scene::Render(const Game& game)
    {
        auto& context3D = game.IndependentResources().Context3D();
        mainCamera_.PrepareForRendering(context3D, game);
        for (const auto& object : m_objects)
        {
            object->Render(context3D, game);
        }
    }
}

