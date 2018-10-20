#include "pch.hpp"
#include "Scene.hpp"
#include "Game.hpp"

namespace dx
{
    SceneBase::SceneBase(Game& game)
        : Device3D{game.IndependentResources().Device3D()}, Predefined{game.Predefined()}
    {}

    SceneBase::~SceneBase() {}
    void SceneBase::Update(const UpdateArgs&, const Game&) {}
    void SceneBase::Render(const Game&) {}
} // namespace dx
