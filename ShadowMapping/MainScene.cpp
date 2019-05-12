#include "Pch.hpp"
#include <DirectXColors.h>
#include "MainScene.hpp"
#include <EasyDx/Systems/SimpleRender.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std::literals;

MainScene::MainScene(dx::Game& game) : dx::SceneBase{game}
{
    BuildCamera();
    BuildLights();
    LoadScene(dx::PredefinedResources::GetInstance());
    m_shadowMapRenderer = std::make_unique<CascadedShadowMappingRenderer>(
        Device3D,
        CascadedShadowMapConfig{dx::Size{1024, 1024}, dx::Size{1008, 985},
                                std::array{0.0f, 0.1f, 0.3f, 0.5f, 1.0f}});

    // InitShadowMapping(game);
}

// void MainScene::InitShadowMapping(dx::Game& game)
//{
//    auto [width, height] =
//    game.GlobalGraphics().GetSwapChain().BufferSize(); for (std::uint32_t
//    i = 0; i < kCascadedCount; ++i)
//    {
//        CD3D11_TEXTURE2D_DESC desc{DXGI_FORMAT_R24G8_TYPELESS, width,
//        height}; desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//        dx::TryHR(Device3D.CreateTexture2D(&desc, {},
//        m_shadowMap.GetAddressOf()));
//    }
//
//    D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc{};
//    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//    dx::TryHR(Device3D.CreateDepthStencilView(m_shadowMap.Get(),
//    &dsDesc));
//    // Device3D.CreateRenderTargetView(m_shadowMap.Get(), &rtDesc,
//    m_shadowMapRt.GetAddressOf());
//}

void MainScene::LoadScene(const dx::PredefinedResources& predefinedRes)
{
    Assimp::Importer importer;
    const fs::path modelPath =
        fs::current_path() / "Models" / "room_thickwalls.obj";
    const auto TryAssimp = [&](auto ptr) {
        if (ptr == nullptr)
            throw std::runtime_error{importer.GetErrorString()};
        return ptr;
    };
    const aiScene* const scene =
        TryAssimp(importer.ReadFile(modelPath.u8string().c_str(),
                                    aiProcessPreset_TargetRealtime_MaxQuality));
    CollectMaterials(*scene, modelPath.parent_path(), predefinedRes,
                     m_materials);
    using namespace DirectX;
    for (const aiMesh* aiMesh_ : dx::GetMeshesInScene(*scene))
    {
        std::shared_ptr<dx::Mesh> mesh =
            dx::ConvertToImmutableMesh(Device3D, *aiMesh_);
        m_objects.push_back(std::make_shared<dx::Object>(
            dx::MeshRenderer{mesh, m_materials[aiMesh_->mMaterialIndex]}));
        m_objects.push_back(std::make_shared<dx::Object>(
            dx::MeshRenderer{mesh, m_materials[aiMesh_->mMaterialIndex]},
            dx::TransformComponent{dx::Transform(
                XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), XMQuaternionIdentity(),
                XMVectorSet(0.0f, 0.0f, 15.0f, 0.0f))}));
       /* m_objects.push_back(std::make_shared<dx::Object>(
            dx::MeshRenderer{mesh, m_materials[aiMesh_->mMaterialIndex]},
            dx::TransformComponent{dx::Transform(
                XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), XMQuaternionIdentity(),
                XMVectorSet(0.0f, 0.0f, 20.0f, 0.0f))}));*/
    }
}

// void MainScene::DrawQuad(ID3D11DeviceContext& context3D, const dx::Mesh&
// mesh,
//                         ID3D11ShaderResourceView& tex,
//                         ID3D11SamplerState& sampler, DirectX::XMVECTOR
//                         color)
//{
//    using namespace dx;
//    // a temp hack
//    auto refCountedTex = wrl::ComPtr<ID3D11ShaderResourceView>{&tex};
//    auto refCountedSampler = wrl::ComPtr<ID3D11SamplerState>{&sampler};
//    auto& pass = m_quadMat->Passes[0];
//    auto& ps = pass.Shaders.PixelShader_.Inputs;
//    ps.Bind("MainTex", refCountedTex);
//    ps.Bind("MainTexSampler", refCountedSampler);
//    DrawMesh(context3D, mesh, *m_quadMat);
//}

void MainScene::PrepareRenderParams(std::vector<dx::RenderNode>& nodes,
                                    dx::GlobalShaderContext& context)
{
    nodes.clear();
    for (const auto& object : m_objects)
    {
        const auto renderer = object->GetComponent<dx::MeshRenderer>();
        if (renderer == nullptr)
            continue;
        nodes.push_back(dx::RenderNode{
            renderer->GetMesh(), renderer->GetMaterial(),
            dx::MatrixFromTransform(
                object->GetComponent<dx::TransformComponent>())});
    }
    auto& camera = MainCamera();
    context.ProjMatrix = camera.GetProjection();
    context.ViewMatrix = camera.GetView();
    context.ViewProjMatrix = context.ViewMatrix * context.ProjMatrix;
    context.EyePos = camera.GetEyePos();
    context.lightCount = Lights().size();
    std::copy(Lights().begin(), Lights().end(), context.lights);
}

// void MainScene::AppendShadowMappingPass(dx::Material& material, const
// dx::Smoothness& smoothness)
//{
//    dx::Pass shadowMappingPass{
//        dx::ShaderCollection{dx::VertexShader{Device3D,
//        dx::AsBytes(SecondPassVSByteCode)},
//                             dx::PixelShader{Device3D,
//                             dx::AsBytes(SecondPassPSByteCode)}}};
//    auto& psInputs = shadowMappingPass.Shaders.PixelShader_.Inputs;
//    psInputs.Bind("ShadowMapSampler", Predefined.GetShadowMapSampler());
//    psInputs.Bind("ShadowMap", dx::Get2DTexView(Device3D,
//    dx::Ref(m_shadowMap))); psInputs.Bind("Texture",
//    Predefined.GetWhite()); psInputs.Bind("Sampler",
//    Predefined.GetDefaultSampler()); auto& perObjectMat =
//    *psInputs.GetCbInfo("PerObjectLightingInfo");
//    perObjectMat.Set("ObjectMaterial", dx::cb::Material{smoothness,
//    true}); material.Passes.push_back(std::move(shadowMappingPass));
//}
//
// void MainScene::PrepareShadowMappingPsCb(ID3D11DeviceContext& context3D,
//                                         const dx::Smoothness&
//                                         smoothness, dx::PixelShader& ps)
//{
//    auto& camera = MainCamera();
//    auto& inputs = ps.Inputs;
//    auto& globalLightingInfo = *inputs.GetCbInfo("GlobalLightingInfo");
//    globalLightingInfo.Set("EyePos", camera.GetEyePos());
//    globalLightingInfo.Set("DirectionalLight",
//    dx::cb::Light{Lights()[0]});
//}

void MainScene::BuildCamera()
{
    auto& camera = MainCamera();
    camera.SetFov(DirectX::XM_PIDIV4);
    camera.Viewport() = {0.0f, 0.0f, 1.0f, 1.0f};
    camera.SetFarZ(1000.0f);
    camera.SetNearZ(1.0f);
    camera.UseDefaultMoveEvents(true);
}

using namespace DirectX;

// void PrepareShadowMapVSCb(dx::VertexShader& vs, const XMMATRIX& world,
//                          const XMMATRIX& lightViewProj, const XMMATRIX&
//                          viewProj, const XMMATRIX& invTransViewProj)
//{
//    auto& inputs = vs.Inputs;
//    auto& cb = *inputs.GetCbInfo("Cb");
//    cb.Set("World", world);
//    cb.Set("LightViewProj", lightViewProj);
//    cb.Set("ViewProj", viewProj);
//    cb.Set("InvTransViewProj", invTransViewProj);
//}

const DirectX::XMFLOAT3 LightDir = {0.57735f, -0.57735f, 0.57735f};

void MainScene::BuildLights()
{
    auto& lights = Lights();
    lights.emplace_back(
        dx::DirectionalLight{{1.0f, 1.0f, 1.0f, 0.0f}, LightDir, true});
}

/*
    0   1
    2   3
*/

// std::shared_ptr<dx::Mesh> MakeQuad(ID3D11Device& device3D,
// gsl::span<dx::PositionType> positions,
//                                   gsl::span<dx::ColorType> colors,
//                                   gsl::span<dx::TexCoordType> uvs)
//{
//    using namespace dx;
//    using gsl::span;
//    auto quadInputLayout = InputLayoutAllocator::Query(QuadDescs);
//    constexpr auto semantics =
//        std::array{VSSemantics::kPosition, VSSemantics::kColor,
//        VSSemantics::kTexCoord0};
//    constexpr auto indices = std::array<dx::ShortIndex, 6>{0, 1, 2, 2, 1,
//    3}; return dx::Mesh::CreateImmutable(device3D,
//    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, span{indices},
//                                     semantics.data(), 4,
//                                     positions.data(), colors.data(),
//                                     uvs.data());
//}

void DrawRect()
{
    using namespace dx;
    auto vs = Shaders::Get(Shaders::kQuadVS);
    auto ps = Shaders::Get(Shaders::kQuadPS);
}

void MainScene::CollectMaterials(
    const aiScene& scene, const fs::path& materialParent,
    const dx::PredefinedResources& predefinedResource,
    std::vector<std::shared_ptr<dx::Material>>& materials)
{
    materials.clear();
    materials.reserve(scene.mNumMaterials);
    for (std::uint32_t i = 0; i < scene.mNumMaterials; ++i)
    {
        const aiMaterial& aiMat = *scene.mMaterials[i];
        const dx::Smoothness smoothness =
            /*dx::SmoothnessFromMaterial(aiMat).value_or*/ (
                dx::kDefaultSmoothness);
        aiString diffuseTexPath;
        wrl::ComPtr<ID3D11ShaderResourceView> mainTexSrv =
            predefinedResource.GetWhite();
        wrl::ComPtr<ID3D11SamplerState> mainTexSampler =
            predefinedResource.GetDefaultSampler();
        if (aiMat.Get(AI_MATKEY_TEXTURE_DIFFUSE(0), diffuseTexPath) ==
            aiReturn_SUCCESS)
        {
            const wrl::ComPtr<ID3D11Texture2D> mainTex = dx::Load2DTexFromFile(
                Device3D, materialParent / diffuseTexPath.C_Str());
            mainTexSrv = dx::Get2DTexView(Device3D, dx::Ref(mainTex));
            CD3D11_SAMPLER_DESC samplerDesc{CD3D11_DEFAULT{}};
            dx::FillAddressModes(aiMat, 0, samplerDesc);
            dx::TryHR(Device3D.CreateSamplerState(
                &samplerDesc, mainTexSampler.GetAddressOf()));
        }
        materials.push_back(dx::MakeBasicLightingMaterial(
            predefinedResource, smoothness, mainTexSrv, mainTexSampler));
    }
}

void MainScene::Render(ID3D11DeviceContext& context3D,
                       dx::GlobalGraphicsContext& gfxContext,
                       const dx::Game& game)
{
    std::vector<dx::RenderNode> renderNodes;
    dx::GlobalShaderContext context;
    PrepareRenderParams(renderNodes, context);
    ID3D11RenderTargetView* const mainRt = gfxContext.MainRt();
    gfxContext.ClearBoth();
    gfxContext.ClearMainRt(DirectX::Colors::White);
    m_shadowMapRenderer->GenerateShadowMap(gfxContext, MainCamera(), Lights(),
                                           gsl::make_span(renderNodes),
                                           context);
     context3D.OMSetRenderTargets(1, &mainRt,
     gfxContext.GetDepthStencil().View());
    ////TODO: sort by material
     for (const dx::RenderNode& node : renderNodes)
    {
        const dx::Material& material = node.material;
        const dx::Mesh& mesh = node.mesh;
        const dx::PassWithShaderInputs& mainPassWithInputs =
        material.mainPass; dx::FillUpShaders(context3D,
        mainPassWithInputs, node.World, nullptr, context);
        dx::DrawMesh(context3D, mesh, mainPassWithInputs.pass);
    }

    gfxContext.GetSwapChain().Present();

    // 1. render the shadow map
    // context3D.OMSetRenderTargets(0, nullptr, m_shadowMapDsView.Get());
    // PrepareVsCb(
    //    context3D, shaders.VertexShader_.Inputs,
    //    DirectX::XMMatrixIdentity(), XMMatrixLookToLH(XMVectorZero(),
    //    dx::Load(LightDir), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
    //    *
    //        perspectiveProj,
    //    XMMatrixOrthographicLH(1.0f,
    //    gfxContext.GetSwapChain().BufferSize().GetAspectRatio(),
    //                           camera.NearZ(), camera.FarZ()));
    //// no need to prepare PS.
    //// PreparePsCb(context3D, shaders.PixelShader_.Inputs, Lights(),
    /// camera);
    // DrawMesh(context3D, mesh, material, 0);
    //// 2 use that shadow map rendering the scene
    //// const auto lights = gsl::make_span(dirLights_);
    // auto pMainRt = gfxContext.MainRt();
    // context3D.OMSetRenderTargets(1, &pMainRt,
    // gfxContext.GetDepthStencil().View()); gfxContext.ClearBoth();
    // gfxContext.ClearMainRt(DirectX::Colors::SkyBlue);

    // auto& [shaders, blending, depthStencil, rs] =
    // m_sceneMaterial->Passes[0];

    // dx::DrawMesh(context3D, *m_sceneMesh, *m_sceneMaterial);
}