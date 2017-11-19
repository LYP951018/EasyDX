#pragma once

#include <d3d11.h>
#include <EasyDx/Scene.hpp>
#include <EasyDx/Common.hpp>
#include <EasyDx/Predefined.hpp>
#include <optional>

struct alignas(16) CbPerObject;
struct alignas(16) CbPerFrame;

struct TessQuad;

class MainScene : public dx::Scene
{
public:
    MainScene(dx::Game& game, dx::Rc<void> args);
    void Update(const dx::UpdateArgs& args) override;
    ~MainScene() override;

private:
    std::unique_ptr<TessQuad> quad_;
    dx::DirectionalLight light_;
};