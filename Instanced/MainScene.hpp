#pragma once

#include <DirectXMath.h>
#include <EasyDx/Fwd.hpp>

struct InstancingVertex;

class MainScene : public dx::SceneBase
{
  public:
    MainScene(dx::Game& game);

  private:
    void InitInstancingBuffer();
    void InitBall();
    void Render(const dx::Game& game) override;
    void BuildCamera();

    std::shared_ptr<dx::Mesh> m_ballMesh;
    std::shared_ptr<dx::Material> m_ballMaterial;
    dx::AlignedVec<InstancingVertex> m_instancingData;
    dx::TypedGpuBuffer<InstancingVertex> m_instancingBuffer;
    std::vector<InstancingVertex> m_visibleBuffer;
};