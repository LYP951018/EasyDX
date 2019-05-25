#include "Pch.hpp"
#if 1
#include "CascadedShadowMappingRenderer.hpp"
#include <DirectXColors.h>

using namespace DirectX;
using namespace dx;

CascadedShadowMappingRenderer::CascadedShadowMappingRenderer(
    ID3D11Device& device3D, const CascadedShadowMapConfig& shadowMapConfig)
    : m_config{shadowMapConfig}, m_partitions{shadowMapConfig.Intervals}
{
    CreateDepthGenerationResources(device3D);
    CreateCsmGenerationResources(device3D, shadowMapConfig.ShadowMapSize);
    CreateSssmRt(device3D, shadowMapConfig.ScreenSpaceTexSize);
}

XMMATRIX OrthographicFromBoundingBox(const BoundingBox& box)
{
    XMFLOAT3 corners[8];
    box.GetCorners(corners);
    const XMFLOAT3& minPoint = corners[2];
    const XMFLOAT3& maxPoint = corners[4];
    return XMMatrixOrthographicOffCenterLH(minPoint.x, maxPoint.x, minPoint.y,
                                           maxPoint.y, minPoint.z, maxPoint.z);
}

DirectX::BoundingBox BoxFromFrutum(const BoundingFrustum& frustum)
{
    XMFLOAT3 corners[8];
    frustum.GetCorners(corners);
    DirectX::BoundingBox box;
    BoundingBox::CreateFromPoints(box, 8, corners, sizeof(XMFLOAT3));
    return box;
}

void CascadedShadowMappingRenderer::GenerateShadowMap(
    const dx::GlobalGraphicsContext& gfxContext, const dx::Camera& camera,
    gsl::span<const dx::Light> lights, gsl::span<const RenderNode> renderNodes,
    const dx::GlobalShaderContext& shaderContext)
{
    ID3D11DeviceContext& context3D = gfxContext.Context3D();

    // first pass: collect depth in view space
    // 为了从 screen space 还原到 world space。
    std::array<ID3D11RenderTargetView* const, 1> nullView = {};
    context3D.OMSetRenderTargets(1, nullView.data(), m_worldDepthView.Get());
   // context3D.ClearRenderTargetView(rt, color.data());
    context3D.ClearDepthStencilView(m_worldDepthView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    RunShadowCaster(renderNodes, shaderContext, context3D);
    BoundingBox viewSpaceSceneAabb;
    BoundingBox::CreateFromPoints(viewSpaceSceneAabb, g_XMNegInfinity,
                                  g_XMInfinity);
    for (const RenderNode& renderNode : renderNodes)
    {
        const BoundingBox& localBoundingBox = renderNode.mesh.GetBoundingBox();
        BoundingBox worldBoundingBox;
        localBoundingBox.Transform(worldBoundingBox,
                                   renderNode.World * camera.GetView());
        BoundingBox::CreateMerged(viewSpaceSceneAabb, viewSpaceSceneAabb,
                                  worldBoundingBox);
    }

    // m_viewSpaceDepthMap contains what we want
    // second pass: CSM
    BoundingFrustum frustum = camera.Frustum();
    const dx::Light& mainLight = lights[0];
    BoundingFrustum lightSpaceFrustum;
    XMMATRIX lightSpaceViewMatrix;
    XMMATRIX lightSpaceProjMatrix;
    XMMATRIX viewToLight;
    switch (mainLight.index())
    {
        case dx::kDirectionalLight:
        {
            auto& directionalLight = std::get<DirectionalLight>(mainLight);
            lightSpaceViewMatrix = XMMatrixLookToLH(
                XMVectorZero(), Load(directionalLight.Direction),
                XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            viewToLight =
                XMMatrixInverse({}, camera.GetView()) * lightSpaceViewMatrix;
            BoundingBox lightSpaceAabb;
            viewSpaceSceneAabb.Transform(lightSpaceAabb, viewToLight);
            lightSpaceProjMatrix = OrthographicFromBoundingBox(lightSpaceAabb);
        }
    }

    frustum.Transform(lightSpaceFrustum, viewToLight);
    // BoundingBox lightSpaceFrustumBox = BoxFromFrutum(lightSpaceFrustum);
    // lightSpaceProjMatrix =
    // OrthographicFromBoundingBox(lightSpaceFrustumBox);
    const std::uint32_t partitionCount = kCascadedCount;
    dx::GlobalShaderContext shaderContextForShadowMapping = shaderContext;
    shaderContextForShadowMapping.ViewMatrix = lightSpaceViewMatrix;
    const float nearZ =
        std::max(frustum.Near,
                 viewSpaceSceneAabb.Center.z - viewSpaceSceneAabb.Extents.z);
    const float farZ =
        viewSpaceSceneAabb.Center.z + viewSpaceSceneAabb.Extents.z;
    const float nearFarDistance = farZ - nearZ;
    const XMFLOAT4 colors[] = {
        XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f},
        XMFLOAT4{0.0f, 1.0f, 0.0f, 1.0f},
        XMFLOAT4{0.0f, 0.0f, 1.0f, 1.0f},
        XMFLOAT4{1.0f, 1.0f, 0.0f, 1.0f},
    };
    D3D11_VIEWPORT viewport{
        0.0f, 0.0f, m_config.ShadowMapSize.Width, m_config.ShadowMapSize.Height, 0, 1
    };;
    context3D.RSSetViewports(1, &viewport);
    for (std::uint32_t i = 0; i < partitionCount; ++i)
    {
        const float low = nearZ + m_partitions[i] * nearFarDistance;
        const float high = nearZ + m_partitions[i + 1] * nearFarDistance;
        m_intervals[i] = low;
        XMMATRIX projMatrix = CalcLightProjMatrix(
            mainLight, frustum, low, high, lightSpaceProjMatrix,
            viewSpaceSceneAabb, viewToLight);
        shaderContextForShadowMapping.ProjMatrix =
            lightSpaceProjMatrix * projMatrix;
        if (!m_lightSpaceFrustum[i])
        {
            m_lightSpaceFrustum[i] = MeshFromFrustum(
                gfxContext.Device3D(), lightSpaceFrustum, colors[i]);
        }
        shaderContextForShadowMapping.ViewProjMatrix =
            shaderContextForShadowMapping.ViewMatrix *
            shaderContextForShadowMapping.ProjMatrix;
        m_lightViewProjs[i] = viewToLight * shaderContextForShadowMapping.ProjMatrix;
        ID3D11RenderTargetView* rt = m_shadowMapRtViews[i].Get();
        context3D.OMSetRenderTargets(1, &rt, m_shadowMapRtDepthStencil.View());
        std::array<float, 4> color = {};
        context3D.ClearRenderTargetView(rt, color.data());
        m_shadowMapRtDepthStencil.ClearBoth(context3D);
        ShaderInputs inputs;
        for (const RenderNode& renderNode : renderNodes)
        {
            // FIXME：如何避免上一个对象设置的 buffer 遗留的问题？
            FillUpShaders(context3D, renderNode.material.shadowCasterPass,
                          renderNode.World, nullptr,
                          shaderContextForShadowMapping);
            DrawMesh(context3D, renderNode.mesh,
                     *renderNode.material.shadowCasterPass.pass);
        }
    }
    /*if (!m_cubePass)
    {
        const auto mappedCso =
            MemoryMappedCso{fs::current_path() / L"CubeVS.cso"};
        m_cubePass = std::make_shared<Pass>(Pass{MakeShaderCollection(
            Shader{gfxContext.Device3D(), mappedCso.Bytes()},
            dx::Shader::FromCompiledCso(gfxContext.Device3D(),
                                        fs::current_path() / L"CubePS.cso"))});
        InputLayoutAllocator::Register(
            gfxContext.Device3D(),
            m_lightSpaceFrustum[0]->GetFullInputElementDesces(),
            mappedCso.Bytes());
    }
    std::array<ID3D11RenderTargetView* const, 1> views = {gfxContext.MainRt()};
    context3D.OMSetRenderTargets(1, views.data(),
                                 gfxContext.GetDepthStencil().View());
    for (const std::shared_ptr<dx::Mesh>& mesh : m_lightSpaceFrustum)
    {
        FillUpShaders(context3D, dx::PassWithShaderInputs{m_cubePass},
                      XMMatrixIdentity(), nullptr, shaderContext);
        DrawMesh(context3D, *mesh, *m_cubePass);
    }*/
    ID3D11RenderTargetView* rts[] = { m_sssmRt.Get() };
    context3D.OMSetRenderTargets(1, rts, nullptr);
    D3D11_VIEWPORT viewport2{
        0.0f, 0.0f, m_config.ScreenSpaceTexSize.Width, m_config.ScreenSpaceTexSize.Height, 0, 1
    };;
    context3D.RSSetViewports(1, &viewport2);
    float color[4] = {};
    context3D.ClearRenderTargetView(m_sssmRt.Get(), color);
    ShaderInputs& inputs = m_collectPass.inputs;
    const XMMATRIX invProj = XMMatrixInverse({}, camera.GetProjection());
    inputs.SetField("InvProj", invProj);
    inputs.SetField("lightSpaceProjs", m_lightViewProjs);
    inputs.SetField("Intervals", m_intervals);
    inputs.Bind("DepthMap", m_depthSrv);
    inputs.Bind("ShadowMapArray", m_shadowMapTexArraySrv);
    inputs.Bind("NearestPointSampler", m_nearestPointSampler);
    FillUpShaders(context3D, m_collectPass,
        DirectX::XMMatrixIdentity(), nullptr,
        shaderContextForShadowMapping);
    DrawMesh(context3D, *m_screenSpaceQuad,
        m_collectPass.pass, &gfxContext.Device3D());
    ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr };
    context3D.PSSetShaderResources(0, 2, srvs);
    context3D.VSSetShaderResources(0, 2, srvs);

    // m_depthTexArray has been filled.
    // last pass: generate screen space shadowmap

    // auto invViewProj = XMMatrixInverse({},
    // XMMatrixTranspose(shaderContext.ViewProjMatrix));

    // dx::ShaderInputs additionalInputs;
    //// viewspace
    // additionalInputs.SetField("lightSpaceProjs", m_lightViewProjs);
    // additionalInputs.SetField("InvProj", m_lightViewProjs);
    // additionalInputs.Bind("DepthTex", m_shadowMapTexArraySrv);
    // additionalInputs.Bind("DepthTexSampler", m_nearestPointSampler);
    // DrawMesh(context3D, *m_quad, m_collectPass);
}

wrl::ComPtr<ID3D11ShaderResourceView>
CascadedShadowMappingRenderer::GetCsmTexArray() const
{
    return m_shadowMapTexArraySrv;
}

void CascadedShadowMappingRenderer::RunShadowCaster(
    gsl::span<const dx::RenderNode>& renderNodes,
    const dx::GlobalShaderContext& shaderContextForShadowMapping,
    ID3D11DeviceContext& context3D)
{
    for (const RenderNode& renderNode : renderNodes)
    {
        const Material& material = renderNode.material;
        if (material.shadowCasterPass.pass)
        {
            FillUpShaders(context3D, material.shadowCasterPass,
                          renderNode.World, nullptr,
                          shaderContextForShadowMapping);
            DrawMesh(context3D, renderNode.mesh,
                     *material.shadowCasterPass.pass);
        }
    }
}

void CascadedShadowMappingRenderer::DrawCube(
    gsl::span<const DirectX::XMFLOAT3> points)
{}

std::shared_ptr<dx::Mesh> CascadedShadowMappingRenderer::MeshFromFrustum(
    ID3D11Device& device3D, const DirectX::BoundingFrustum& frustum,
    const XMFLOAT4& color)
{
    XMFLOAT3 corners[8];
    frustum.GetCorners(corners);
    const ShortIndex indices[] = {3, 0, 1, 1, 2, 3, 7, 4, 5, 5, 6, 7,
                                  7, 4, 0, 0, 3, 7, 2, 1, 5, 5, 6, 2,
                                  4, 0, 1, 0, 1, 5, 7, 3, 2, 3, 2, 6};
    const XMFLOAT4 colors[] = {color, color, color, color,
                               color, color, color, color};
    const VSSemantics semantics[] = {VSSemantics::kPosition,
                                     VSSemantics::kColor};
    const DxgiFormat formats[] = {DxgiFormat::R32G32B32Float,
                                  DxgiFormat::R32G32B32A32Float};
    const std::uint32_t semanticsIndices[] = {0, 0};
    /*
            4 0
            7 3
    */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc;
    dx::FillInputElementsDesc(inputDesc, gsl::make_span(semantics),
                              gsl::make_span(formats),
                              gsl::make_span(semanticsIndices));

    return Mesh::CreateImmutable(device3D, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
                                 gsl::make_span(indices), semantics, 8,
                                 std::move(inputDesc), corners, colors);
}

DirectX::XMMATRIX CascadedShadowMappingRenderer::CalcLightProjMatrix(
    const dx::Light& light, DirectX::BoundingFrustum& existingFrustum,
    float low, float high, const XMMATRIX& lightProjMatrix,
    const DirectX::BoundingBox& viewSpaceAabb, const XMMATRIX& viewToLight)
{
    switch (light.index())
    {
        case kDirectionalLight:
        {
            // BUG:
            //上面的 viewSpaceSceneAabb 是物体求出的
            // AABB，视锥角落的点可能不在这里面，导致变换到 homo space
            //外面。
            existingFrustum.Near = low;
            existingFrustum.Far = high;
            XMFLOAT3 frustumCorners[8];
            XMFLOAT3 boxCorners[8];
            XMFLOAT3 corners[8];
            BoundingBox frustumBox;
            existingFrustum.GetCorners(frustumCorners);
            BoundingBox::CreateFromPoints(frustumBox, 8, frustumCorners,
                                          sizeof(XMFLOAT3));
            frustumBox.GetCorners(frustumCorners);
            viewSpaceAabb.GetCorners(boxCorners);
            /*
            const auto select = [&](int index) {
                    const XMFLOAT3& boxCorner = boxCorners[index];
                    const XMFLOAT3& frustumCorner = frustumCorners[index];
                    const XMVECTOR minBox = XMLoadFloat3(&boxCorner);
                    const XMVECTOR minFrustum = XMLoadFloat3(&frustumCorner);
                    return XMVectorMin(g_BoxOffset[index] * minBox,
            g_BoxOffset[index] * minFrustum);
                    

            };

            BoundingBox newBox;
            BoundingBox::CreateFromPoints(newBox, select(2), select(4));
            newBox.GetCorners(corners);*/
            for (int i = 0; i < 8; ++i)
            {
                const float* offset = g_BoxOffset[i];
                const XMFLOAT3& boxCorner = boxCorners[i];
                const XMFLOAT3& frustumCorner = frustumCorners[i];
                const auto select = [&](int index) {
                    const float selector = offset[index];
                    return selector *
                           std::min(selector * (&boxCorner.x)[index],
                                    selector * (&frustumCorner.x)[index]);
                };
                corners[i] = {select(0), select(1), select(2)};
            }

            BoundingBox newBox;
            BoundingBox::CreateFromPoints(newBox, 8, corners, sizeof(XMFLOAT3));
            newBox.Transform(newBox, viewToLight);
            newBox.GetCorners(corners);
            // for (int i = 0; i < 8; ++i)
            //{
            //	const XMFLOAT3& frustumCorner = frustumCorners[i];
            //	const XMFLOAT3& boxCorner = boxCorners[i];
            //	const XMVECTOR minimum =
            //XMVectorMin(XMLoadFloat3(&frustumCorner),
            //XMLoadFloat3(&boxCorner)); 	XMStoreFloat3(corners + i, minimum);
            //}

            // Then, each corner point p of the camera’s
            // frustum slice is projected into p h = PMp in the light’s
            // homogeneous space.
            for (XMFLOAT3& corner : corners)
            {
                XMFLOAT4 position = {corner.x, corner.y, corner.z, 1.0f};
                XMVECTOR loaded = XMLoadFloat4(&position);
                loaded = XMVector4Transform(loaded, lightProjMatrix);
                loaded /= XMVectorGetW(loaded);
                XMStoreFloat3(&corner, loaded);
            }
            BoundingBox lightHomoBox;
            BoundingBox::CreateFromPoints(lightHomoBox, 8, corners,
                                          sizeof(XMFLOAT3));
            lightHomoBox.GetCorners(corners);
            const XMFLOAT3& minPoint = corners[2];
            const XMFLOAT3& maxPoint = corners[4];
            return XMMatrixOrthographicOffCenterLH(minPoint.x, maxPoint.x,
                                                   minPoint.y, maxPoint.y,
                                                   minPoint.z, maxPoint.z);
        }
        default:
            break;
    }
}

void CascadedShadowMappingRenderer::CreateCsmGenerationResources(
    ID3D11Device& device3D, dx::Size size)
{
    D3D11_TEXTURE2D_DESC depthTexArrayDesc{};
    depthTexArrayDesc.ArraySize = kCascadedCount;
    depthTexArrayDesc.Width = size.Width;
    depthTexArrayDesc.Height = size.Height;
    depthTexArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexArrayDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    depthTexArrayDesc.Format = DXGI_FORMAT_R32_FLOAT;
    depthTexArrayDesc.MipLevels = 1;
    depthTexArrayDesc.SampleDesc.Count = 1;
    TryHR(device3D.CreateTexture2D(&depthTexArrayDesc, nullptr,
                                   m_depthTexArray.GetAddressOf()));

    D3D11_SHADER_RESOURCE_VIEW_DESC rsvDesc{};
    rsvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    rsvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
    D3D11_TEX2D_ARRAY_SRV& tex2DArray = rsvDesc.Texture2DArray;
    tex2DArray.ArraySize = kCascadedCount;
    tex2DArray.MipLevels = 1;
    tex2DArray.MostDetailedMip = 0;
    tex2DArray.FirstArraySlice = 0;
    TryHR(device3D.CreateShaderResourceView(
        m_depthTexArray.Get(), &rsvDesc,
        m_shadowMapTexArraySrv.GetAddressOf()));
    D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc{};
    rtViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
    rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    D3D11_TEX2D_ARRAY_RTV& rtTexArray = rtViewDesc.Texture2DArray;
    rtTexArray.MipSlice = 0;
    rtTexArray.ArraySize = 1;
    for (unsigned i = 0; i < kCascadedCount; ++i)
    {
        rtViewDesc.Texture2DArray.FirstArraySlice =
            D3D11CalcSubresource(0, i, 1);
        TryHR(device3D.CreateRenderTargetView(
            m_depthTexArray.Get(), &rtViewDesc,
            m_shadowMapRtViews[i].GetAddressOf()));
    }
    m_shadowMapRtDepthStencil = DepthStencil{device3D, size};
}

void CascadedShadowMappingRenderer::CreateSssmRt(ID3D11Device& device3D,
                                                 dx::Size size)
{
    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = size.Width;
    texDesc.Height = size.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.SampleDesc.Count = 1;
    TryHR(device3D.CreateTexture2D(&texDesc, nullptr,
                                   m_screenSpaceShadowMap.GetAddressOf()));
    D3D11_RENDER_TARGET_VIEW_DESC rtDesc{};
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    TryHR(device3D.CreateRenderTargetView(m_screenSpaceShadowMap.Get(), &rtDesc,
                                          m_sssmRt.GetAddressOf()));
     const ShortIndex quadIndices[] = {0, 1, 2, 2, 1, 3};
     const PositionType quadPositions[] = {
         MakePosition(-1.0f, 1.0f, 1.0f),
         MakePosition(1.0f, 1.0f, 1.0f),
         MakePosition(-1.0f, -1.0f, 1.0f),
         MakePosition(1.0f, -1.0f, 1.0f),
     };
     const TexCoordType quadTexCoords[] = {
         MakeTexCoord(0.0f, 0.0f), MakeTexCoord(1.0f, 0.0f),
         MakeTexCoord(0.0f, 1.0f), MakeTexCoord(1.0f, 1.0f)}; 
     VSSemantics semantics[] = {
         VSSemantics::kPosition, VSSemantics::kTexCoord
     };
     DxgiFormat formats[] = {
         DxgiFormat::R32G32B32A32Float,
         DxgiFormat::R32G32Float
     };
     std::uint32_t indices[] = { 0, 0 };
     std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementsDesces;
     FillInputElementsDesc(inputElementsDesces, semantics, formats,
         indices);
     m_screenSpaceQuad = Mesh::CreateImmutable(
         device3D, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, gsl::span(quadIndices),
         semantics, 4, std::move(inputElementsDesces),
     quadPositions, quadTexCoords);

     m_collectPass.pass = std::make_shared<dx::Pass>(dx::Pass
     {
         MakeShaderCollection(
             dx::Shader::FromCompiledCso(device3D, fs::current_path() /
                                                       "ShadowCollectVS.cso"),
             dx::Shader::FromCompiledCso(device3D, fs::current_path() /
                                                       L"ShadowCollectPS.cso"))
     });
     CD3D11_SAMPLER_DESC samplerDesc{ CD3D11_DEFAULT{} };
     samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
     TryHR(device3D.CreateSamplerState(&samplerDesc, m_nearestPointSampler.GetAddressOf()));
}

void CascadedShadowMappingRenderer::CreateDepthGenerationResources(
    ID3D11Device& device3D)
{
    // first pass: collect depth
    CD3D11_TEXTURE2D_DESC depthTexDesc{DXGI_FORMAT_R32G8X24_TYPELESS,
                                       m_config.ScreenSpaceTexSize.Width,
                                       m_config.ScreenSpaceTexSize.Height, 1, 1};
    depthTexDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
    TryHR(device3D.CreateTexture2D(&depthTexDesc, nullptr,
                                   m_worldSpaceDepthMap.GetAddressOf()));
    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{
        D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT_S8X24_UINT};
    TryHR(device3D.CreateDepthStencilView(m_worldSpaceDepthMap.Get(),
                                          &depthStencilViewDesc,
                                          m_worldDepthView.GetAddressOf()));
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{
        D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS};
    TryHR(device3D.CreateShaderResourceView(
        m_worldSpaceDepthMap.Get(), &srvDesc, m_depthSrv.GetAddressOf()));
}

dx::Pass
CascadedShadowMappingRenderer::MakeCollectionPass(ID3D11Device& device3D)
{
    throw std::runtime_error{"fuck"};
    /*using namespace dx;
    return Pass{ShaderCollection{
        Shader::FromCompiledCso(device3D, fs::current_path() /
    "SecondPassVS.hlsl"), Shader::FromCompiledCso(device3D,
    fs::current_path() / "SecondPassPS.hlsl")}};*/
}
#endif