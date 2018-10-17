#pragma once

#include <DirectXMath.h>
#include <EasyDx/Fwd.hpp>


class MainScene : public dx::SceneBase
{
  public:
    MainScene(dx::Game&);

  private:
    //void Update(const dx::UpdateArgs& args, const dx::Game& game) override;
    void Render(const dx::Game& game) override;
    void BuildLights();
    void BuildObjects();
    void InitReflectedMaterial();
    void BuildCamera();
    void LoadTextures();
    std::unique_ptr<dx::Object> MakeBall() const;
    std::unique_ptr<dx::Object> MakeMirror() const;
    std::unique_ptr<dx::Object> MakeWall() const;
    std::unique_ptr<dx::Object> MakeFloor() const;

    wrl::ComPtr<ID3D11Texture2D> m_checkBoardTex, m_brick01Tex, m_iceTex;
    ID3D11Device& m_device3D;
    const dx::PredefinedResources& m_predefined;
    std::unique_ptr<dx::Object> m_floor, m_wall, m_mirror, m_ball;
    std::shared_ptr<dx::Material> m_reflectedMaterial;
};