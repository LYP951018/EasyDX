#include "pch.hpp"
#include "Scene.hpp"
#include "Game.hpp"

namespace dx
{
    SceneBase::SceneBase(Game& game)
        : Device3D{game.GlobalGraphics().Device3D()}
    {}

    SceneBase::~SceneBase() {}

    void SceneBase::Update(const UpdateArgs&, const Game&) {}

    void SceneBase::Render(
        [[maybe_unused]] ID3D11DeviceContext& context3D,
        [[maybe_unused]] GlobalGraphicsContext& globalGraphicsResources,
        [[maybe_unused]] const Game& game)
    {}
} // namespace dx
