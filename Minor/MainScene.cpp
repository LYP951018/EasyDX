#include <EasyDx/One.hpp>
#include "MainScene.hpp"
#include <DirectXColors.h>
#include <numeric>

MainScene::MainScene(const dx::Game& game, dx::Rc<void> args)
    : Scene{game},
    resized_{AddResize()}
{
    auto& device = game.GetDevice3D();
    BuildLights();
    BuildRoom(device, game.GetPredefined());
    auto& camera = GetMainCamera();
    const float theta = 1.24f * DirectX::XM_PI;
    const float phi = 0.42f * DirectX::XM_PI;
    const float radius = 12.0f;
    float x = radius * sinf(phi) * cosf(theta);
    float z = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);
    camera.SetLookAt({ x, y, z }, {}, { 0.0f, 1.0f, 0.0f });
}


void MainScene::BuildRoom(ID3D11Device& device, const dx::Predefined& predefined)
{
    using namespace DirectX;
    const auto roomSmoothness = dx::Smoothness{
        XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
            XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }, 16.0f};

    {
        auto floorTex = dx::Load2DTexFromFile(device, fs::current_path() / "Tex" / "checkboard.dds");

        const dx::SimpleVertex FloorVertices[] = {
            { { -3.5f, 0.0f, -10.0f },{ 0.0f, 1.0f, 0.0f },{},{ 0.0f, 4.0f } },
            { { -3.5f, 0.0f,   0.0f },{ 0.0f, 1.0f, 0.0f },{},{ 0.0f, 0.0f } },
            { { 7.5f, 0.0f,   0.0f },{ 0.0f, 1.0f, 0.0f },{},{ 4.0f, 0.0f } },
            { { -3.5f, 0.0f, -10.0f },{ 0.0f, 1.0f, 0.0f },{},{ 0.0f, 4.0f } },
            { { 7.5f, 0.0f,   0.0f },{ 0.0f, 1.0f, 0.0f },{},{ 4.0f, 0.0f } },
            { { 7.5f, 0.0f, -10.0f },{ 0.0f, 1.0f, 0.0f },{},{ 4.0f, 4.0f } }
        };

        const std::uint16_t FloorIndices[] = {
            0, 1, 2, 3, 4, 5
        };
        floor_ = dx::BasicObject{
            dx::BasicRenderable{device, predefined, FloorVertices, FloorIndices, dx::Get2DTexView(device, dx::Ref(floorTex)), predefined.GetRepeatSampler()},
            roomSmoothness,
            DirectX::XMMatrixIdentity()
        };
    }
    
    {
        auto wallTex = dx::Load2DTexFromFile(device, fs::current_path() / "Tex" / "brick01.dds");

        const dx::SimpleVertex WallVertices[] = {
            { { -3.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f },{},{ 0.0f, 2.0f } },
            { { -3.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 0.0f } },
            { { -2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.5f, 0.0f } },
            { { -3.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 2.0f } },
            { { -2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.5f, 0.0f } },
            { { -2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.5f, 2.0f } },
            { { 2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 2.0f } },
            { { 2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 0.0f } },
            { { 7.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 2.0f, 0.0f } },
            { { 2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 2.0f } },
            { { 7.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 2.0f, 0.0f } },
            { { 7.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 2.0f, 2.0f } },
            { { -3.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 1.0f } },
            { { -3.5f, 6.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 0.0f } },
            { { 7.5f, 6.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 6.0f, 0.0f } },
            { { -3.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 1.0f } },
            { { 7.5f, 6.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 6.0f, 0.0f } },
            { { 7.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 6.0f, 1.0f } },
        };

        std::uint16_t wallIndices[18];
        std::iota(std::begin(wallIndices), std::end(wallIndices), 0);

        //WTF??
        wall_ = dx::BasicObject{
            dx::BasicRenderable{ device, predefined, WallVertices, wallIndices, dx::Get2DTexView(device, dx::Ref(wallTex)), predefined.GetRepeatSampler()},
            roomSmoothness,
            DirectX::XMMatrixIdentity()
        };
    }

    {
        auto mirrorTex = dx::Load2DTexFromFile(device, fs::current_path() / "Tex" / "ice.dds");

        auto mirrorSmoothness = dx::Smoothness{
            XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.5f },
            XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
            XMFLOAT4{},
            16.0f
        };
        const dx::SimpleVertex MirrorVertices[] = {
            { { -2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 1.0f } },
            { { -2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 0.0f } },
            { { 2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 1.0f, 0.0f } },
            { { -2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 0.0f, 1.0f } },
            { { 2.5f, 4.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 1.0f, 0.0f } },
            { { 2.5f, 0.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{},{ 1.0f, 1.0f } }
        };

        const std::uint16_t MirrorIndices[] = {
            0, 1, 2, 3, 4, 5
        };

        mirror_ = dx::BasicObject{
            dx::BasicRenderable{ device, predefined, MirrorVertices, MirrorIndices, dx::Get2DTexView(device, dx::Ref(mirrorTex)), {} },
            mirrorSmoothness,
            DirectX::XMMatrixIdentity()
        };
    }
    {
        auto ballSmoothness = dx::Smoothness{
            XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
            XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
            XMFLOAT4{},
            16.0f
        };
        dx::SimpleCpuMesh mesh;
        dx::MakeUVSphere(1.0f, 30, 30, mesh);
        const auto view = mesh.Get();

        dx::Transformation transform;
        transform.Translation.z = -2.5f;
        transform.Translation.y = 2.f;

        sphere_ = dx::BasicObject{
            dx::BasicRenderable{device, predefined, mesh.Get(), {}, {}},
            ballSmoothness,
            dx::ComputeWorld(transform)
        };

        auto reflection = DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
        auto reflected = dx::ComputeWorld(transform) * reflection;
        reflectedSphere_ = dx::BasicObject{
            dx::BasicRenderable{device, predefined, view, {}, {}},
            ballSmoothness,
            reflected
        };

        const auto shadowPlane = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const auto toMainLight = DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&std::get<dx::DirectionalLight>(dirLights_[0]).Direction));
        const auto shadowMatrix = DirectX::XMMatrixShadow(shadowPlane, toMainLight);
        sphereShadow_ = sphere_;
        auto& shadowTransform = sphereShadow_.Transform;
        DirectX::XMStoreFloat4x4(&shadowTransform, sphereShadow_.GetWorld() * shadowMatrix * DirectX::XMMatrixTranslation(0.0f, 0.001f, 0.0f));
    }
}

void MainScene::BuildLights()
{
    std::array<dx::DirectionalLight, 3> dirLights = {};
    dirLights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    dirLights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    dirLights[2].Direction = { 0.0f, -0.707f, -0.707f };
    for (auto& light : dirLights)
    {
        light.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        light.Enabled = true;
    }
    //dirLights[0].Enabled = false;
    dirLights[1].Enabled = false;
    dirLights[2].Enabled = false;
    std::copy(dirLights.begin(), dirLights.end(), dirLights_.begin());
}


void MainScene::Update(const dx::UpdateArgs& args)
{
    const auto& game = GetGame();
    game.GetMainWindow()->Clear(DirectX::Colors::White);

    auto& context = args.Context3D;
    const auto& camera = GetMainCamera();
    const auto lights = gsl::make_span(dirLights_);

    const auto drawContext = dx::BasicDrawContext{
        context, camera, lights
    };

    const auto& predefined = game.GetPredefined();

    //1. render floor, wall, sphere normally.
    {
        dx::UpdateAndDraw(drawContext, wall_);
        dx::UpdateAndDraw(drawContext, floor_);
        dx::UpdateAndDraw(drawContext, sphere_);
    }
    
    //2. Render mirror. stencil buffer only
    {
        auto noWriteToRT = predefined.GetNoWriteToRT();
        auto stencilAlways = predefined.GetStencilAlways();
        context.OMSetBlendState(noWriteToRT.Get(), nullptr, static_cast<UINT>(-1));
        context.OMSetDepthStencilState(stencilAlways.Get(), 1);
        dx::UpdateAndDraw(drawContext, mirror_);
        context.OMSetDepthStencilState(0, 0);
        context.OMSetBlendState(0, nullptr, 0xffffffff);
    }

    //3. Draw the reflected sphere.
    {
        context.RSSetState(predefined.GetCullClockwise());
        context.OMSetDepthStencilState(predefined.GetDrawnOnly().Get(), 1);
        auto cachedLights = dirLights_;
        auto reflection = DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
        for (auto& light : dirLights_)
        {
            auto& direction = std::get<dx::DirectionalLight>(light).Direction;
            auto dirF4 = dx::MakeDirection(direction);
            auto reflected = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&dirF4), reflection);
            DirectX::XMStoreFloat3(&direction, reflected);
        }
        dx::UpdateAndDraw(drawContext, reflectedSphere_);
        context.RSSetState(nullptr);
        context.OMSetDepthStencilState(nullptr, 0);
        dirLights_ = cachedLights;
    }

    //4. Draw the mirror with alpha blending.
    {
        float blendFactor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
        context.OMSetBlendState(predefined.GetTransparent().Get(), blendFactor, static_cast<UINT>(-1));
        dx::UpdateAndDraw(drawContext, mirror_);
        
    }

    //5. Draw the shadow.
    {
        context.OMSetDepthStencilState(predefined.GetNoDoubleBlending().Get(), 0);
        dx::UpdateAndDraw(drawContext, sphereShadow_);
        context.OMSetBlendState({}, {}, static_cast<UINT>(-1));
        context.OMSetDepthStencilState({}, 0);
    }
}

MainScene::~MainScene()
{
}

dx::EventHandle<dx::WindowResizeEvent> MainScene::AddResize()
{
    auto& camera = GetMainCamera();
    auto mainWindow = GetGame().GetMainWindow();
    return mainWindow->WindowResize.Add([&](dx::ResizeEventArgs& e) {
        camera.SetProjection(DirectX::XM_PIDIV4, e.NewSize.GetAspectRatio(), 0.01f, 1000.f);
        camera.MainViewport = {
            0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
            0.f, 1.f
        };
    });
}