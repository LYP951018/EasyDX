#include "Pch.hpp"
#include "MainScene.hpp"
#include "InstancingVS.hpp"
#include <EasyDx/Systems/SimpleRender.hpp>
#include <DirectXColors.h>

struct alignas(16) InstancingVertex
{
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX InvTransWorld;
};

constexpr auto kInstancingVertexUnits =
    std::array{dx::MakeVertex(dx::VSSemantics::kPosition, 0),
               dx::MakeVertex(dx::VSSemantics::kNormal, 1),
               dx::MakeVertex(dx::VSSemantics::kTexCoord, 2),
               MATRIX_VERTEX_UNITS("WORLDMATRIX", 3),
               MATRIX_VERTEX_UNITS("INVTRANSWORLDMATRIX", 3)};

constexpr auto kInstancingVertexDescs =
    dx::MakeDescArray(kInstancingVertexUnits);

void GenerateInstancingData(dx::AlignedVec<InstancingVertex>& instancingData,
                            std::int32_t n)
{
    instancingData.resize(n * n * n);
    const float dist = 1.0f;
    for (std::int32_t i = 0; i < n; ++i)
    {
        for (std::int32_t j = 0; j < n; ++j)
        {
            for (std::int32_t k = 0; k < n; ++k)
            {
                auto& data = instancingData[i * n * n + j * n + k];
                data.World = DirectX::XMMatrixTranslation(
                    static_cast<float>(i) * dist, static_cast<float>(j) * dist,
                    static_cast<float>(k) * dist);
                data.InvTransWorld = DirectX::XMMatrixInverse(
                    {}, DirectX::XMMatrixTranspose(data.World));
            }
        }
    }
}

using namespace DirectX;

void Culling(const DirectX::BoundingFrustum& frustum,
             gsl::span<const dx::PositionType> mesh, const XMMATRIX& view,
             gsl::span<const InstancingVertex> transforms,
             std::vector<InstancingVertex>& visibleParts,
             ID3D11DeviceContext& context3D, dx::GpuBuffer& instancingBuffer)
{
    visibleParts.clear();
    const auto invView = XMMatrixInverse({}, view);
    BoundingBox aabb;
    DirectX::BoundingFrustum localFrustum;
    std::copy_if(
        transforms.begin(), transforms.end(), std::back_inserter(visibleParts),
        [&](const InstancingVertex& v) {
            BoundingBox::CreateFromPoints(aabb, mesh.size(), mesh.data(),
                                          sizeof(dx::PositionType));
            const auto inv = invView * XMMatrixInverse({}, v.World);
            frustum.Transform(localFrustum, inv);
            return localFrustum.Contains(aabb) == ContainmentType::CONTAINS;
        });
    dx::UpdateWithDiscard(context3D, dx::Ref(instancingBuffer),
                          gsl::make_span(visibleParts));
}

MainScene::MainScene(dx::Game& game) : dx::SceneBase{game}
{
    BuildCamera();
    InitInstancingBuffer();
    InitBall();
}

void MainScene::BuildCamera()
{
    auto& lights = Lights();
    auto& camera = MainCamera();
    std::array<dx::DirectionalLight, 3> dirLights = {};
    dirLights[0].Direction = {0.57735f, -0.57735f, 0.57735f};
    dirLights[1].Direction = {-0.57735f, -0.57735f, 0.57735f};
    dirLights[2].Direction = {0.0f, -0.707f, -0.707f};
    for (auto& light : dirLights)
    {
        light.Color = {1.0f, 1.0f, 1.0f, 1.0f};
        light.Enabled = true;
        lights.emplace_back(light);
    }
    camera.UseDefaultMoveEvents(true);
    camera.Viewport() = dx::Rect{0.0f, 0.0f, 1.0f, 1.0f};
    camera.SetFarZ(1000.0f);
    camera.SetNearZ(1.0f);
    camera.SetFov(DirectX::XM_PIDIV4);
}

void MainScene::InitInstancingBuffer()
{
    GenerateInstancingData(m_instancingData, 10);
    m_instancingBuffer =
        dx::MakeDynamicVertexBuffer(Device3D, gsl::make_span(m_instancingData));
}

void MainScene::InitBall()
{
    dx::LoadedMesh sphereMesh;
    dx::MakeUVSphere(0.2f, 10, 10, sphereMesh);
    auto inputLayout = dx::MakeInputLayout(Device3D, kInstancingVertexDescs,
                                           dx::AsBytes(InstancingVSByteCode));
    using namespace gsl;
    using namespace dx;
    m_ballMesh = dx::Mesh::CreateImmutable(
        Device3D, std::move(inputLayout), make_span(sphereMesh.Indices),
        make_span(sphereMesh.Positions), make_span(sphereMesh.Normals),
        make_span(sphereMesh.TexCoords));
    auto ps = Predefined.GetBasicPS();
    PresetupBasicPsCb(ps.Inputs, Predefined,
                      dx::Smoothness{DirectX::XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f},
                                     DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f},
                                     DirectX::XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f},
                                     DirectX::XMFLOAT4{}, 16.0f});
    m_ballMaterial = std::make_shared<Material>(Material{MakeVec(Pass{
        ShaderCollection{VertexShader{Device3D, AsBytes(InstancingVSByteCode)},
                         std::move(ps)}})});
}

void MainScene::Render(const dx::Game& game)
{
    auto& dependentGraphics = game.DependentResources();
    auto& [swapChain, depthStencilBuffer] = dependentGraphics;
    auto& independent = game.IndependentResources();
    auto& context3D = independent.Context3D();
    dependentGraphics.Bind(context3D);
    const auto& camera = MainCamera();
    depthStencilBuffer.ClearBoth(context3D);
    swapChain.Front().Clear(context3D, DirectX::Colors::White);
    context3D.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    using namespace dx::systems;
    auto& shaders = m_ballMaterial->Passes[0].Shaders;
    shaders.VertexShader_.Inputs.Set("TfMatrices", "ViewProj",
                                     camera.GetView() * camera.GetProjection());
    PreparePsCb(context3D, shaders.PixelShader_.Inputs,
                gsl::make_span(Lights()), camera);
    const std::uint32_t instancingVertexSize =
        static_cast<std::uint32_t>(sizeof(InstancingVertex));
    Culling(camera.Frustum(), m_ballMesh->Positions(), camera.GetView(),
            m_instancingData, m_visibleBuffer, context3D, m_instancingBuffer);
    dx::DrawMeshInstancing(context3D, *m_ballMesh, *m_ballMaterial,
                           m_visibleBuffer.size(),
                           dx::SingleAsSpan(m_instancingBuffer),
                           dx::SingleAsSpan(instancingVertexSize));
}
