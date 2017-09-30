#include "MainScene.hpp"
#include <EasyDx/One.hpp>
#include <DirectXColors.h>
#include <numeric>

MainScene::MainScene(const dx::Game& game, dx::Rc<void> args)
    : Scene{game},
    sphere_{dx::MakeShared<dx::GameObject>()},
    mirror_{dx::MakeShared<dx::GameObject>()},
    wall_{dx::MakeShared<dx::GameObject>()},
    floor_{dx::MakeShared<dx::GameObject>()}
{
    auto& device = game.GetDevice3D();
    BuildRoom(device, game.GetPredefined());
    BuildLights();
    auto& camera = GetMainCamera();
    const auto theta = 1.24 * DirectX::XM_PI;
    const auto phi = 0.42 * DirectX::XM_PI;
    const auto radius = 12.0f;
    float x = radius*sinf(phi)*cosf(theta);
    float z = radius*sinf(phi)*sinf(theta);
    float y = radius*cosf(phi);
    camera.SetLookAt({ x, y, z }, {}, { 0.0f, 1.0f, 0.0f });
}


void MainScene::BuildRoom(ID3D11Device& device, const dx::Predefined& predefined)
{
    using namespace DirectX;
    const auto roomSmoothness = dx::MakeShared<dx::Smoothness>(
        XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
        XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
        XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
        XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }, 16.0f);

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
        floor_ = dx::MakeBasicGameObject(device, predefined, 
            gsl::make_span(FloorVertices), 
            gsl::make_span(FloorIndices), 
            roomSmoothness, {},
            dx::MakeShared<dx::Texture>(device, floorTex, predefined.GetDefaultSampler()));
    }
    
    {
        auto wallTex = dx::Load2DTexFromFile(device, fs::current_path() / "Tex" / "brick01.dds");

        const dx::SimpleVertex WallVertices[] = {
            { { -3.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 2.0f }, {}, { 0.0f, 0.0f }},
            { { -3.5f, 4.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, {}, { 0.0f, 0.0f }},
            { { -2.5f, 4.0f, 0.0f }, { -1.0f, 0.5f, 0.0f }, {}, { 0.0f, 0.0f }},
            { { -3.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 2.0f }, {}, { 0.0f, 0.0f } },
            { { -2.5f, 4.0f, 0.0f }, { -1.0f, 0.5f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { -2.5f, 0.0f, 0.0f }, { -1.0f, 0.5f, 2.0f }, {}, { 0.0f, 0.0f } },
            { { 2.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 2.0f }, {}, { 0.0f, 0.0f } },
            { { 2.5f, 4.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 4.0f, 0.0f }, { -1.0f, 2.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { 2.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 2.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 4.0f, 0.0f }, { -1.0f, 2.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 0.0f, 0.0f }, { -1.0f, 2.0f, 2.0f }, {}, { 0.0f, 0.0f } },
            { {-3.5f, 4.0f, 0.0f }, { -1.0f, 0.0f, 1.0f }, {}, { 0.0f, 0.0f } },
            { { -3.5f, 6.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 6.0f, 0.0f }, { -1.0f, 6.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { -3.5f, 4.0f, 0.0f }, { -1.0f, 0.0f, 1.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 6.0f, 0.0f }, { -1.0f, 6.0f, 0.0f }, {}, { 0.0f, 0.0f } },
            { { 7.5f, 4.0f, 0.0f }, { -1.0f, 6.0f, 1.0f }, {}, { 0.0f, 0.0f } },
        };

        std::uint16_t wallIndices[18];
        std::iota(std::begin(wallIndices), std::end(wallIndices), 0);

        //WTF??
        wall_ = dx::MakeBasicGameObject(device, 
            predefined, 
            gsl::make_span(WallVertices), 
            gsl::make_span<const std::uint16_t>(wallIndices),
            roomSmoothness, {}, dx::MakeShared<dx::Texture>(device, wallTex, predefined.GetDefaultSampler()));
    }

    {
        auto mirrorTex = dx::Load2DTexFromFile(device, fs::current_path() / "Tex" / "ice.dds");

        auto mirrorSmoothness = dx::MakeShared<dx::Smoothness>(
            XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.5f },
            XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
            XMFLOAT4{},
            16.0f
            );
        const dx::SimpleVertex MirrorVertices[] = {
            { { -2.5f, 0.0f, 0.0f },{ -1.0f, 0.0f, 1.0f },{},{ 0.0f, 0.0f } },
            { { -2.5f, 4.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },{},{ 0.0f, 0.0f } },
            { { 2.5f, 4.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{},{ 0.0f, 0.0f } },
            { { -2.5f, 0.0f, 0.0f },{ -1.0f, 0.0f, 1.0f },{},{ 0.0f, 0.0f } },
            { { 2.5f, 4.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{},{ 0.0f, 0.0f } },
            { { 2.5f, 0.0f, 0.0f },{ -1.0f, 1.0f, 1.0f },{},{ 0.0f, 0.0f } }
        };

        const std::uint16_t MirrorIndices[] = {
            1, 2, 3, 4, 5, 6
        };

        mirror_ = dx::MakeBasicGameObject(device, predefined, 
            gsl::make_span(MirrorVertices), 
            gsl::make_span(MirrorIndices), std::move(mirrorSmoothness), {},
            dx::MakeShared<dx::Texture>(device, mirrorTex, predefined.GetDefaultSampler()));
    }
    
    {
        auto ballSmoothness = dx::MakeShared<dx::Smoothness>(
            XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f },
            XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f },
            XMFLOAT4{ 0.4f, 0.4f, 0.4f, 1.0f },
            XMFLOAT4{},
            16.0f
            );
        dx::SimpleCpuMesh mesh;
        dx::MakeUVSphere(10.0f, 30, 30, mesh);
        const auto view = mesh.Get();
        sphere_ = dx::MakeBasicGameObject(device, predefined, view.Vertices, view.Indices, ballSmoothness);

        auto reflection = DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
        reflectedSphere_ = dx::MakeBasicGameObject(device, predefined, view.Vertices, view.Indices, ballSmoothness);
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
    std::copy(dirLights.begin(), dirLights.end(), dirLights_);
}


void MainScene::Update(const dx::UpdateArgs& args)
{
    const auto& game = GetGame();
    game.GetMainWindow()->Clear(DirectX::Colors::Blue);
    auto& context = args.Context3D;
    const auto& predefined = game.GetPredefined();

    //1. render floor, wall, sphere normally.
    {
        dx::BasicSystem(context, args, *wall_);
        dx::BasicSystem(context, args, *floor_);
        dx::BasicSystem(context, args, *sphere_);
    }
    
    //2. Render mirror. stencil buffer only
    {
        auto noWriteToRT = predefined.GetNoWriteToRT();
        auto stencilAlways = predefined.GetStencilAlways();
        float blendFactor[4] = {};
        context.OMSetBlendState(noWriteToRT.Get(), blendFactor, static_cast<UINT>(-1));
        context.OMSetDepthStencilState(stencilAlways.Get(), 1);
        dx::BasicSystem(context, args, *mirror_);
        context.OMSetDepthStencilState(0, 0);
        context.OMSetBlendState(0, blendFactor, 0xffffffff);
    }

    //3. Draw the reflected sphere.
    {
        context.RSSetState(predefined.GetCullClockwise());
        context.OMSetDepthStencilState(predefined.GetDrawnOnly().Get(), 1);
        //auto cachedLights = std::array<Light
        dx::BasicSystem(context, args, *reflectedSphere_);
        context.RSSetState(nullptr);
        context.OMSetDepthStencilState(nullptr, 0);
    }

    //4. Draw the mirror with alpha blending.
    {
        float blendFactor[4] = {};
        context.OMSetBlendState(predefined.GetTransparent().Get(), blendFactor, static_cast<UINT>(-1));
        dx::BasicSystem(context, args, *mirror_);
    }

    //5. Draw the shadow.
    {

    }
}