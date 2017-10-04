#include <EasyDx/One.hpp>
#include "MainScene.hpp"
#include "Rotator.hpp"
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "Cb.hpp"
#include "CbUpdator.hpp"

using namespace dx;

MainScene::MainScene(const dx::Game& game, std::shared_ptr<void> arg)
    : resized_{AddResize()}
{
    lightGlobal_ = dx::PointLight{
        { 2.f, 3.f, 6.f },
        { 1.f, 1.f, 1.f, 1.f },
        { 1.f, 0.08f, 0.0f },
        1000.f,
        true
    };
    using namespace DirectX;
    auto& camera = GetMainCamera();
    const auto eye = XMFLOAT3{ 0.0f, 4.0f, -10.0f };
    const auto at = XMFLOAT3{ 0.f, 1.f, 0.f };
    const auto up = XMFLOAT3{ 0.f, 1.f, 0.f };
    camera.SetLookAt(eye, at, up);
    SetupBall(game.GetDevice3D());
    /* const dx::SimpleVertex vertices[] = {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), {}, { 0.f, 0.f } },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), {}, { 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), {},{ 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), {},{ 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), {},{ 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), {},{ 0.f, 1.f } },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) ,{},{ 1.f, 0.f } },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),{},{ 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),{},{ 0.f, 0.f } },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),{},{ 0.f, 1.f } },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),{},{ 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),{},{ 1.f, 1.f } },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),{},{ 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),{},{ 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),{},{ 1.f, 0.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),{},{ 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),{},{ 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),{},{ 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),{},{ 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),{},{ 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),{},{ 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),{},{ 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),{},{ 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),{},{ 1.f, 1.f } },
    };

    const std::uint16_t indices[] = {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };*/
}

void MainScene::Update(const dx::UpdateArgs& args)
{
    auto& ball = *ball_;
    //1, update frame constant buffer.
    auto& cbPerFrame = cpuCbPerFrame_->Data();
    cbPerFrame.Dlight.FromPoint(lightGlobal_);
    cbPerFrame.EyePos = dx::MakePosition(GetMainCamera().GetEyePos());
    cpuCbPerFrame_->Flush(args.Context3D);
    //2, invoke all behaviors
    dx::BehaviorSystem(ball, args);
    //3, render.
    dx::RenderSystem(args.Context3D, ball);
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

void MainScene::SetupBall(ID3D11Device& device3D)
{
    auto ball = dx::MakeShared<dx::GameObject>();
    auto vs = dx::VertexShader::FromByteCode(device3D, dx::AsBytes(TheVertexShader), dx::SimpleVertex::GetLayout());
    auto ps = dx::PixelShader::FromByteCode(device3D, dx::AsBytes(ThePixelShader));

    auto [cpuCbPerObject, gpuCbPerObject] = dx::MakeCb<data::CbPerObject>(device3D);
    std::tie(cpuCbPerFrame_, gpuCbPerFrame_) = dx::MakeCb<data::CbPerFrame>(device3D);
    for (auto cb : { gpuCbPerObject, gpuCbPerFrame_})
    {
        vs.Cbs.push_back(cb);
        ps.Cbs.push_back(cb);
    }

    auto smoothness = dx::MakeShared<dx::Smoothness>(
        DirectX::XMFLOAT4{ 0.07568f, 0.61424f, 0.07568f, 1.0f },
        DirectX::XMFLOAT4{ 0.07568f, 0.61424f, 0.07568f, 1.0f },
        DirectX::XMFLOAT4{ 0.07568f, 0.61424f, 0.07568f, 1.0f },
        DirectX::XMFLOAT4{}, 76.8f);

    dx::SimpleCpuMesh meshData;
    //dx::MakeUVSphere(1.f, 20, 20, meshData);
    dx::MakeCylinder(1.f, 0.5f, 2.f, 20, 20, meshData);
    auto mesh = GpuMesh{ device3D, meshData.Get() };
    auto update = MakeShared<dx::FuncBehavior>(Rotator{lightGlobal_});
    auto cbUpdator = MakeShared<dx::FuncBehavior>(CbPerObjectUpdator{ GetMainCamera() }, Behavior::kCbUpdate);
    auto renderable = MakeShared<dx::Renderable>(std::move(vs), std::move(ps), std::move(mesh), cpuCbPerObject);
    ball->AddComponents(std::move(smoothness), std::move(renderable));
    ball->AddBehaviors(std::move(update), std::move(cbUpdator));
    ball_ = ball;
}
