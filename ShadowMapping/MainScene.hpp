#pragma once

#include <DirectXMath.h>
#include <EasyDx/Fwd.hpp>
#include "CascadedShadowMappingRenderer.hpp"

class MainScene : public dx::SceneBase
{
  public:
    MainScene(dx::Game& game);
    void BuildLights();
    void BuildCamera();

  private:
    void LoadScene(const dx::PredefinedResources& predefinedRes);

    void
    CollectMaterials(const aiScene& scene, const fs::path& materialParent,
                     const dx::PredefinedResources& predefinedResource,
                     std::vector<std::shared_ptr<dx::Material>>& materials);
    void Render(ID3D11DeviceContext& context3D,
                dx::GlobalGraphicsContext& gfxContext, const dx::Game& game);
    // void InitShadowMapping(dx::Game& game);

    void PrepareRenderParams(std::vector<dx::RenderNode>& nodes,
                             dx::GlobalShaderContext& context);
    std::vector<std::shared_ptr<dx::Object>> m_objects;
    std::vector<std::shared_ptr<dx::Material>> m_materials;
    std::shared_ptr<dx::Mesh> m_quad;
    std::unique_ptr<CascadedShadowMappingRenderer> m_shadowMapRenderer;
};
