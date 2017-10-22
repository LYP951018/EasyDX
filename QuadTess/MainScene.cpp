#include <EasyDx/One.hpp>
#include "MainScene.hpp"
#include <DirectXColors.h>
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "HullShader.hpp"
#include "DomainShader.hpp"

struct alignas(16) CbPerObject
{
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX WorldViewProj;
};

struct alignas(16) CbPerFrame
{
    DirectX::XMFLOAT3 EyePos;
    float Padding;
};

struct Vertex
{
    DirectX::XMFLOAT4 Pos;

    static constexpr D3D11_INPUT_ELEMENT_DESC Desc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
};

MainScene::MainScene(const dx::Game& game, dx::Rc<void> args)
    : dx::Scene{game},
    shaders_{game.GetDevice3D(), TheVertexShader, gsl::make_span(Vertex::Desc), ThePixelShader, TheHullShader, TheDomainShader},
    cbPerObject_{ dx::aligned_unique<CbPerObject>() },
    cbPerFrame_{ dx::aligned_unique<CbPerFrame>() },
    gpucbPerObject_{dx::MakeConstantBuffer<CbPerObject>(game.GetDevice3D())},
    gpuCbPerFrame_{dx::MakeConstantBuffer<CbPerFrame>(game.GetDevice3D())}
{
    
    auto& camera = GetMainCamera();
    const auto theta = 1.3f * DirectX::XM_PI;
    const auto phi = 0.2f * DirectX::XM_PI;
    const auto radius = 50.0f;
    const auto x = radius * std::sin(phi) * std::cos(theta);
    const auto z = radius * std::sin(phi) * std::sin(theta);
    const auto y = radius * std::cos(phi);
    camera.SetLookAt({ 0.0f, 50.0f ,50.0f }, {}, DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f });
    BuildQuad(game.Device3D());
}

MainScene::~MainScene() {}

void MainScene::BuildQuad(ID3D11Device& device)
{
    const Vertex vertices[] = {
        DirectX::XMFLOAT4{-10.0f, 0.0f, 10.0f, 1.0f},
        DirectX::XMFLOAT4{10.0f, 0.0f, 10.0f, 1.0f},
        DirectX::XMFLOAT4{-10.0f, 0.0f, -10.0f, 1.0f},
        DirectX::XMFLOAT4{10.0f, 0.0f, -10.0f, 1.0f}
    };
    
    const std::uint16_t indices[] = { 0, 1, 2, 3 };

    mesh_ = dx::GpuMesh{ device, vertices, indices };
    dx::TryHR(device.CreateInputLayout(Vertex::Desc, 1, TheVertexShader, std::size(TheVertexShader), layout_.GetAddressOf()));
}

void MainScene::Update(const dx::UpdateArgs& args)
{
    const auto& game = GetGame();
    game.MainWindow().Clear(DirectX::Colors::White);
    auto& context = args.Context3D;
    const auto& camera = GetMainCamera();
    const auto world = DirectX::XMMatrixIdentity();
    const auto view = camera.GetView();
    const auto projection = camera.GetProjection();
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    //context.IASetInputLayout()
    cbPerObject_->World = world;
    cbPerObject_->WorldViewProj = world * view * projection;
    cbPerFrame_->EyePos = camera.GetEyePos();
    dx::SetShaders(context, shaders_.Get());
    dx::UpdateCb(context, gpucbPerObject_, *cbPerObject_);
    dx::UpdateCb(context, gpuCbPerFrame_, *cbPerFrame_);
    dx::SetupGpuMesh(context, mesh_.Get());
    const std::array<dx::Ptr<ID3D11Buffer>, 2> cbs = {
        gpucbPerObject_.GpuCb.Get(),
        gpuCbPerFrame_.GpuCb.Get()
    };
    context.DSSetConstantBuffers(0, cbs.size(), cbs.data());
    context.HSSetConstantBuffers(0, cbs.size(), cbs.data());
    context.RSSetState(game.GetPredefined().GetWireFrameOnly());
    context.DrawIndexed(mesh_.Get().IndexCount, 0, 0);
}