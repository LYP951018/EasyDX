#include "pch.hpp"
#include "MainScene.hpp"
#include <EasyDx/Systems/SimpleRender.hpp>
#include <DirectXColors.h>
#include <numeric>

using gsl::span;

std::unique_ptr<dx::Object> MainScene::MakeFloor() const
{
    using namespace DirectX;

    const dx::PositionType positions[] = {{-3.5f, 0.0f, -10.0f}, {-3.5f, 0.0f, 0.0f},
                                          {7.5f, 0.0f, 0.0f},    {-3.5f, 0.0f, -10.0f},
                                          {7.5f, 0.0f, 0.0f},    {7.5f, 0.0f, -10.0f}};
    const dx::VectorType normals[] = {
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    };
    const dx::TexCoordType texCoords[] = {{0.0f, 4.0f}, {0.0f, 0.0f}, {4.0f, 0.0f},
                                          {0.0f, 4.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}};
    const dx::ShortIndex indices[] = {0, 1, 2, 3, 4, 5};
    const dx::Smoothness smoothness = {
        XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f},
        XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, 16.0f};

    return dx::MakeObjectWithDefaultRendering(
        Device3D, Predefined,
        dx::PosNormTexVertexInput{span{positions}, span{normals}, span{texCoords}, span{indices}},
        smoothness, dx::Get2DTexView(Device3D, dx::Ref(m_checkBoardTex)), Predefined.GetRepeatSampler());
}

std::unique_ptr<dx::Object> MainScene::MakeWall() const
{
    const dx::PositionType positions[] = {
        {-3.5f, 0.0f, 0.0f}, {-3.5f, 4.0f, 0.0f}, {-2.5f, 4.0f, 0.0f}, {-3.5f, 0.0f, 0.0f},
        {-2.5f, 4.0f, 0.0f}, {-2.5f, 0.0f, 0.0f}, {2.5f, 0.0f, 0.0f},  {2.5f, 4.0f, 0.0f},
        {7.5f, 4.0f, 0.0f},  {2.5f, 0.0f, 0.0f},  {7.5f, 4.0f, 0.0f},  {7.5f, 0.0f, 0.0f},
        {-3.5f, 4.0f, 0.0f}, {-3.5f, 6.0f, 0.0f}, {7.5f, 6.0f, 0.0f},  {-3.5f, 4.0f, 0.0f},
        {7.5f, 6.0f, 0.0f},  {7.5f, 4.0f, 0.0f}};
    const dx::VectorType normals[] = {
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}};
    const dx::TexCoordType texCoords[] = {
        {0.0f, 2.0f}, {0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 2.0f}, {0.5f, 0.0f}, {0.5f, 2.0f},
        {0.0f, 2.0f}, {0.0f, 0.0f}, {2.0f, 0.0f}, {0.0f, 2.0f}, {2.0f, 0.0f}, {2.0f, 2.0f},
        {0.0f, 1.0f}, {0.0f, 0.0f}, {6.0f, 0.0f}, {0.0f, 1.0f}, {6.0f, 0.0f}, {6.0f, 1.0f},
    };
    std::uint16_t indices[18];
    std::iota(std::begin(indices), std::end(indices), 0);
    using namespace DirectX;

    const auto smoothness =
        dx::Smoothness{XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f},
                       XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, 16.0f};

    return dx::MakeObjectWithDefaultRendering(
        Device3D, Predefined,
        dx::PosNormTexVertexInput{span{positions}, span{normals}, span{texCoords}, span{indices}},
        smoothness,
        dx::Get2DTexView(Device3D, dx::Ref(m_brick01Tex)), Predefined.GetRepeatSampler());
}

std::unique_ptr<dx::Object> MainScene::MakeMirror() const
{
    using namespace DirectX;

    const dx::PositionType positions[] = {{-2.5f, 0.0f, 0.0f}, {-2.5f, 4.0f, 0.0f},
                                          {2.5f, 4.0f, 0.0f},  {-2.5f, 0.0f, 0.0f},
                                          {2.5f, 4.0f, 0.0f},  {2.5f, 0.0f, 0.0f}};

    const dx::VectorType normals[] = {
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
    };

    const dx::TexCoordType texCoords[] = {{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
                                          {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};

    const std::uint16_t indices[] = {0, 1, 2, 3, 4, 5};

    auto smoothness =
        dx::Smoothness{XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 0.5f},
                       XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{}, 16.0f};

    return dx::MakeObjectWithDefaultRendering(
        Device3D, Predefined,
        dx::PosNormTexVertexInput{span{positions}, span{normals}, span{texCoords}, span{indices}},
        smoothness, dx::Get2DTexView(Device3D, dx::Ref(m_iceTex)));
}

std::unique_ptr<dx::Object> MainScene::MakeBall() const
{
    using namespace DirectX;

    dx::ModelResultUnit sphereMesh;
    dx::MakeUVSphere(1.0f, 30.0f, 30.0f, sphereMesh);

    auto smoothness =
        dx::Smoothness{XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 0.5f},
                       XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{}, 16.0f};

    auto ball = dx::MakeObjectWithDefaultRendering(
        Device3D, Predefined,
        sphereMesh,
        smoothness, Predefined.GetWhite());

    dx::Transform transform;
    transform.SetPosition({0.0f, 2.0f, -2.5f});
    ball->AddComponent<dx::TransformComponent>(std::move(transform));
    return ball;
};

MainScene::MainScene(dx::Game& game)
    : dx::SceneBase{game}
{
    auto& resources = game.IndependentResources();
    auto& device = resources.Device3D();
    BuildLights();
    BuildCamera();
    BuildObjects();
}

void MainScene::BuildObjects()
{
    LoadTextures();
    m_ball = MakeBall();
    m_floor = MakeFloor();
    m_wall = MakeWall();
    m_mirror = MakeMirror();
    InitReflectedMaterial();
}

void MainScene::InitReflectedMaterial()
{
    using namespace DirectX;

    m_reflectedMaterial = dx::MakeBasicLightingMaterial(
        Predefined,
        // the same smoothness with sphere
        dx::Smoothness{XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f}, XMFLOAT4{1.0f, 1.0f, 1.0f, 0.5f},
                       XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{}, 16.0f},
        Predefined.GetWhite());
    auto& [shaders, blending, depthStencil, rasterizerState] = m_reflectedMaterial->Passes[0];
    rasterizerState = Predefined.GetCullClockwise();
    depthStencil.StencilState = Predefined.GetDrawnOnly();
    depthStencil.StencilRef = 1;
}

void MainScene::BuildCamera()
{
    auto& camera = MainCamera();
    const float theta = 1.24f * DirectX::XM_PI;
    const float phi = 0.42f * DirectX::XM_PI;
    const float radius = 12.0f;
    float x = radius * sinf(phi) * cosf(theta);
    float z = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);
    camera.SetLookAt({x, y, z}, {}, {0.0f, 1.0f, 0.0f});
    camera.SetNearZ(1.0f);
    camera.SetFarZ(1000.0f);
    camera.SetFov(DirectX::XM_PIDIV4);
    camera.UseDefaultMoveEvents(true);
    auto& vp = camera.Viewport();
    vp.Right = 1.0f;
    vp.Bottom = 1.0f;
}

void MainScene::BuildLights()
{
    std::array<dx::DirectionalLight, 3> dirLights = {};
    dirLights[0].Direction = {0.57735f, -0.57735f, 0.57735f};
    dirLights[1].Direction = {-0.57735f, -0.57735f, 0.57735f};
    dirLights[2].Direction = {0.0f, -0.707f, -0.707f};
    for (auto& light : dirLights)
    {
        light.Color = {1.0f, 1.0f, 1.0f, 1.0f};
        light.Enabled = true;
    }
    auto& lights = Lights();
    for (auto& dirLight : dirLights)
    {
        lights.emplace_back(std::move(dirLight));
    }
}

void MainScene::LoadTextures()
{
    m_checkBoardTex =
        dx::Load2DTexFromDdsFile(Device3D, fs::current_path() / "Tex" / "checkboard.dds");
    m_brick01Tex = dx::Load2DTexFromDdsFile(Device3D, fs::current_path() / "Tex" / "brick01.dds");
    m_iceTex = dx::Load2DTexFromDdsFile(Device3D, fs::current_path() / "Tex" / "ice.dds");
}

void MainScene::Render(const dx::Game& game)
{
    auto& dependentGraphics = game.DependentResources();
    auto& [swapChain, depthStencilBuffer] = dependentGraphics;
    auto& independent = game.IndependentResources();
    auto& context3D = independent.Context3D();
    dependentGraphics.Bind(context3D);
    const auto& camera = MainCamera();
    // const auto lights = gsl::make_span(dirLights_);
    depthStencilBuffer.ClearBoth(context3D);
    swapChain.Front().Clear(context3D, DirectX::Colors::White);
    context3D.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    const auto& predefined = game.Predefined();

    using namespace dx::systems;

    const auto render = [&](const dx::Object& object) {
        SimpleRenderSystem(context3D, *this, object);
    };

    auto& mirrorMat = m_mirror->GetComponent<dx::MeshRenderer>()->GetMaterial();

    // 1. render floor, wall, sphere normally.
    {
        render(*m_floor);
        render(*m_wall);
        render(*m_ball);
    }

    // 2. Render mirror. stencil buffer only
    {
        auto& [shaders, blending, depthStencil, rasterizerState] = mirrorMat.Passes[0];
        blending.BlendState = predefined.GetNoWriteToRT();
        blending.SampleMask = static_cast<UINT>(-1);
        depthStencil.StencilState = predefined.GetStencilAlways();
        depthStencil.StencilRef = 1;
        render(*m_mirror);
    }

    // 3. Draw the reflected sphere.
    // rendering without an object
    {
        const auto meshRenderer = m_ball->GetComponent<dx::MeshRenderer>();
        auto sharedMesh = meshRenderer->SharedMesh();
        auto& material = meshRenderer->GetMaterial();
        auto lights = Lights();
        auto reflectionMatrix =
            DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
        for (auto& light : lights)
        {
            auto& direction = std::get<dx::DirectionalLight>(light).Direction;
            auto dirF4 = dx::MakeDirection4(direction);
            auto reflected =
                DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&dirF4), reflectionMatrix);
            DirectX::XMStoreFloat3(&direction, reflected);
        }
        auto& transform = m_ball->GetComponent<dx::TransformComponent>()->GetTransform();
        auto& shaders = m_reflectedMaterial->Passes[0].Shaders;
        dx::systems::PrepareVsCb(context3D, shaders.VertexShader_.Inputs,
                                 transform.Matrix() * reflectionMatrix, gsl::make_span(lights), camera);
        dx::systems::PreparePsCb(context3D, shaders.PixelShader_.Inputs, gsl::make_span(lights),
                                 camera);
        dx::DrawMesh(context3D, *sharedMesh, *m_reflectedMaterial);
    }

    // 4. Draw the mirror with alpha blending.
    {
        auto& blending = mirrorMat.Passes[0].Blending;
        blending.BlendFactor = {0.5f, 0.5f, 0.5f, 1.0f};
        blending.BlendState = predefined.GetTransparent();
        blending.SampleMask = static_cast<UINT>(-1);
        render(*m_mirror);
    }

    //// 5. Draw the shadow.
    //{
    //    context3D.OMSetDepthStencilState(predefined.GetNoDoubleBlending().Get(), 0);
    //    dx::UpdateAndDraw(drawContext, sphereShadow_);
    //    context3D.OMSetBlendState({}, {}, static_cast<UINT>(-1));
    //    context3D.OMSetDepthStencilState({}, 0);
    //}

    // swapChain.Present();
}