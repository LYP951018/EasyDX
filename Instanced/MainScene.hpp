#pragma once

#include <EasyDx/Scene.hpp>


struct InstancedObject

class MainScene : public dx::Scene
{
public:
    MainScene(dx::Game&, dx::Rc<void> args);
    void Update(const dx::UpdateArgs&) override;
    ~MainScene() override;

private:
    wrl::ComPtr<ID3D11Buffer> instancingBuffer_;
};
