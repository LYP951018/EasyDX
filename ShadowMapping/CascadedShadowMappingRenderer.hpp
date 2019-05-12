#pragma once

inline static constexpr std::uint32_t kCascadedCount = 4;

template<typename T>
using CascadedArray = std::array<T, kCascadedCount>;

struct CascadedShadowMapConfig
{
    dx::Size ShadowMapSize;
    dx::Size ScreenSpaceTexSize;
    std::array<float, kCascadedCount + 1> Intervals;
};

class CascadedShadowMappingRenderer
{
  public:
    CascadedShadowMappingRenderer(
        ID3D11Device& device3D, const CascadedShadowMapConfig& shadowMapConfig);

    void GenerateShadowMap(const dx::GlobalGraphicsContext& gfxContext,
                           const dx::Camera& camera,
                           gsl::span<const dx::Light> lights,
                           gsl::span<const dx::RenderNode> renderNodes,
                           const dx::GlobalShaderContext& shaderContext);

    wrl::ComPtr<ID3D11ShaderResourceView> GetCsmTexArray() const;

  private:
    DirectX::XMMATRIX
    CalcLightProjMatrix(const dx::Light& light,
                        DirectX::BoundingFrustum& existingFrustum, float low,
                        float high, const DirectX::XMMATRIX& lightProjMatrix,
                        const DirectX::BoundingBox& lightSpaceViewAabb,
                        const DirectX::XMMATRIX& viewToLight);
    void CreateCsmGenerationResources(ID3D11Device& device3D, dx::Size size);
    void CreateSssmRt(ID3D11Device& device3D, dx::Size size);
    void CreateDepthGenerationResources(ID3D11Device& device3D);
    dx::Pass MakeCollectionPass(ID3D11Device& device3D);
    // dx::Pass MakeCollectionPass(ID3D11Device& device3D);
    void RunShadowCaster(
        gsl::span<const dx::RenderNode>& renderNodes,
        const dx::GlobalShaderContext& shaderContextForShadowMapping,
        ID3D11DeviceContext& context3D);
    void DrawCube(gsl::span<const DirectX::XMFLOAT3> points);
    std::shared_ptr<dx::Mesh>
    MeshFromFrustum(ID3D11Device& device3D,
                    const DirectX::BoundingFrustum& frustum,
                    const DirectX::XMFLOAT4& color);
    std::array<float, kCascadedCount + 1> m_partitions;

    // first depth pass
    wrl::ComPtr<ID3D11Texture2D> m_worldSpaceDepthMap;
    wrl::ComPtr<ID3D11DepthStencilView> m_worldDepthView;
    wrl::ComPtr<ID3D11ShaderResourceView> m_depthSrv;

    // CSM pass
    wrl::ComPtr<ID3D11Texture2D> m_depthTexArray;
    wrl::ComPtr<ID3D11ShaderResourceView> m_shadowMapTexArraySrv;
    dx::DepthStencil m_shadowMapRtDepthStencil;
    CascadedArray<wrl::ComPtr<ID3D11RenderTargetView>> m_shadowMapRtViews;
    wrl::ComPtr<ID3D11SamplerState> m_nearestPointSampler;
    std::array<std::shared_ptr<dx::Mesh>, kCascadedCount> m_lightSpaceFrustum;
    std::shared_ptr<dx::Pass> m_cubePass;
    // dx::Pass m_generateLightSpaceDepthPass;

    // screen space pass
    std::array<DirectX::XMMATRIX, kCascadedCount> m_lightViewProjs;
    wrl::ComPtr<ID3D11Texture2D> m_screenSpaceShadowMap;
    wrl::ComPtr<ID3D11RenderTargetView> m_sssmRt;
    dx::PassWithShaderInputs m_collectPass;
    CascadedShadowMapConfig m_config;
    std::shared_ptr<dx::Mesh> m_screenSpaceQuad;
    std::array<float, kCascadedCount> m_intervals;
};

DirectX::XMMATRIX MatrixFromTransform(dx::TransformComponent* transform);
