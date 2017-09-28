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
        floor_ = dx::MakeBasicGameObject(device, predefined, gsl::make_span(FloorVertices), gsl::make_span(FloorIndices), roomSmoothness);
    }
    
    {
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
            roomSmoothness);
    }

    {
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

        mirror_ = dx::MakeBasicGameObject(device, predefined, gsl::make_span(MirrorVertices), gsl::make_span(MirrorIndices), std::move(mirrorSmoothness));
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
        //TODO: reflect the lighing. changing culling direction
        dx::BasicSystem(context, args, *reflectedSphere_);
    }

    //4. Draw the mirror with alpha blending.
    {

    }

    //5. Draw the shadow.
    {

    }
}