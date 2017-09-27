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
        floor_ = dx::MakeBasicGameObject(device, predefined, gsl::make_span(FloorVertices), gsl::make_span(FloorIndices));
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
        wall_ = dx::MakeBasicGameObject(device, predefined, gsl::make_span(WallVertices), gsl::make_span<const std::uint16_t>(wallIndices));
    }

    {
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

        mirror_ = dx::MakeBasicGameObject(device, predefined, gsl::make_span(MirrorVertices), gsl::make_span(MirrorIndices));
    }
    
    {
        dx::SimpleCpuMesh mesh;
        dx::MakeUVSphere(10.0f, 30, 30, mesh);
        const auto view = mesh.Get();
        sphere_ = dx::MakeBasicGameObject(device, predefined, view.Vertices, view.Indices);
    }
}


void MainScene::Update(const dx::UpdateArgs& args)
{
    const auto& game = GetGame();
    game.GetMainWindow()->Clear(DirectX::Colors::Blue);


}