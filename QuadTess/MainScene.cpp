#include <EasyDx/One.hpp>
#include "MainScene.hpp"
#include <DirectXColors.h>
#include <d3d11.h>
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "HullShader.hpp"
#include "DomainShader.hpp"
#include <numeric>

struct alignas(16) CbPerObject
{
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX WorldViewProj;
};

struct alignas(16) CbPerFrame
{
    DirectX::XMFLOAT3 EyePos;
};

struct Vertex
{
    DirectX::XMFLOAT3 Pos;

    static constexpr D3D11_INPUT_ELEMENT_DESC Desc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};

    static gsl::span<const D3D11_INPUT_ELEMENT_DESC> GetDesc() noexcept
    {
        return gsl::make_span(Desc);
    }
};

namespace dx
{
    template<>
    struct is_vertex<::Vertex> : std::true_type
    {};
} // namespace dx

struct TessRenderable
{
    dx::GpuMesh Mesh;
    dx::VertexShader<Vertex> VS;
    dx::SimpleLightingPS PS;
    dx::HullShader<CbPerObject, CbPerFrame> HS;
    dx::DomainShader<CbPerObject, CbPerFrame> DS;
};

struct TessQuad
{
    TessRenderable Pipeline_;
    dx::Smoothness Smoothness;
    DirectX::XMMATRIX World;
};

TessQuad MakeQuad(ID3D11Device& device,
                  const dx::PredefinedResources& predefined)
{
    const Vertex vertices[] = {
        {{1.0f, -0.5f, 0.0f}},         {{1.0f, -0.5f, 0.5f}},
        {{0.5f, -0.3536f, 1.354f}},    {{0.0f, -0.3536f, 1.354f}},
        {{1.0f, -0.1667f, 0.0f}},      {{1.0f, -0.1667f, 0.5f}},
        {{0.5f, -0.1179f, 1.118f}},    {{0.0f, -0.1179f, 1.118f}},
        {{1.0f, 0.1667f, 0.0f}},       {{1.0f, 0.1667f, 0.5f}},
        {{0.5f, 0.1179f, 0.8821f}},    {{0.0f, 0.1179f, 0.8821f}},
        {{1.0f, 0.5f, 0.0f}},          {{1.0f, 0.5f, 0.5f}},
        {{0.5f, 0.3536f, 0.6464f}},    {{0.0f, 0.3536f, 0.6464f}},
        {{0.0f, -0.3536f, 1.354f}},    {{-0.5f, -0.3536f, 1.354f}},
        {{-1.5f, 0.0f, 0.5f}},         {{-1.5f, 0.0f, 0.0f}},
        {{0.0f, -0.1179f, 1.118f}},    {{-0.5f, -0.1179f, 1.118f}},
        {{-1.167f, 0.0f, 0.5f}},       {{-1.167f, 0.0f, 0.0f}},
        {{0.0f, 0.1179f, 0.8821f}},    {{-0.5f, 0.1179f, 0.8821f}},
        {{-0.8333f, 0.0f, 0.5f}},      {{-0.8333f, 0.0f, 0.0f}},
        {{0.0f, 0.3536f, 0.6464f}},    {{-0.5f, 0.3536f, 0.6464f}},
        {{-0.5f, 0.0f, 0.5f}},         {{-0.5f, 0.0f, 0.0f}},
        {{-1.5f, 0.0f, 0.0f}},         {{-1.5f, 0.0f, -0.5f}},
        {{-0.5f, 0.3536f, -1.354f}},   {{0.0f, 0.3536f, -1.354f}},
        {{-1.167f, 0.0f, 0.0f}},       {{-1.167f, 0.0f, -0.5f}},
        {{-0.5f, 0.1179f, -1.118f}},   {{0.0f, 0.1179f, -1.118f}},
        {{-0.8333f, 0.0f, 0.0f}},      {{-0.8333f, 0.0f, -0.5f}},
        {{-0.5f, -0.1179f, -0.8821f}}, {{0.0f, -0.1179f, -0.8821f}},
        {{-0.5f, 0.0f, 0.0f}},         {{-0.5f, 0.0f, -0.5f}},
        {{-0.5f, -0.3536f, -0.6464f}}, {{0.0f, -0.3536f, -0.6464f}},
        {{0.0f, 0.3536f, -1.354f}},    {{0.5f, 0.3536f, -1.354f}},
        {{1.0f, 0.5f, -0.5f}},         {{1.0f, 0.5f, 0.0f}},
        {{0.0f, 0.1179f, -1.118f}},    {{0.5f, 0.1179f, -1.118f}},
        {{1.0f, 0.1667f, -0.5f}},      {{1.0f, 0.1667f, 0.0f}},
        {{0.0f, -0.1179f, -0.8821f}},  {{0.5f, -0.1179f, -0.8821f}},
        {{1.0f, -0.1667f, -0.5f}},     {{1.0f, -0.1667f, 0.0f}},
        {{0.0f, -0.3536f, -0.6464f}},  {{0.5f, -0.3536f, -0.6464f}},
        {{1.0f, -0.5f, -0.5f}},        {{1.0f, -0.5f, 0.0f}},
    };

    std::uint16_t indices[64];
    std::iota(std::begin(indices), std::end(indices), 0);

    using namespace DirectX;
    return TessQuad{
        TessRenderable{dx::GpuMesh{device, vertices, indices},
                       {device, dx::AsBytes(TheVertexShader)},
                       predefined.GetBasicPS(),
                       {device, dx::AsBytes(TheHullShader)},
                       {device, dx::AsBytes(TheDomainShader)}},
        dx::Smoothness{XMFLOAT4{0.5f, 0.5f, 0.5f, 1.0f},
                       XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f},
                       XMFLOAT4{0.4f, 0.4f, 0.4f, 1.0f}, XMFLOAT4{}, 16.0f},
        // DirectX::XMMatrixIdentity()
        DirectX::XMMatrixRotationX(DirectX::XM_PI / 4.0f) *
            DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV4)
        // DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f,
        // 0.0f, 0.0f, 0.0f), -DirectX::XM_PI / 2.0f))
    };
}

MainScene::MainScene(dx::Game& game, dx::Rc<void> args)
    : dx::Scene{game}, quad_{std::make_unique<TessQuad>(
                           MakeQuad(game.IndependentResources().Device3D(),
                                    game.Predefined()))},
      light_{{1.0f, 0.0f, 0.0f, 1.0f}, {-1.0f, -1.5f, 3.5f}, true}
{
    auto& camera = GetMainCamera();
    camera.SetLookAt({1.0f, 1.5f, -3.5f}, {},
                     DirectX::XMFLOAT3{0.0f, 1.0f, 0.0f});
    AddCameraMovement();
}

MainScene::~MainScene() {}

void MainScene::Update(const dx::UpdateArgs& args)
{
    const auto& game = GetGame();
    auto& resources = game.IndependentResources();
    game.MainWindow().Clear(DirectX::Colors::White);
    auto& context = args.Context3D;
    const auto& camera = GetMainCamera();
    auto [renderable, smoothness, world] = *quad_;
    const auto view = camera.GetView();
    const auto projection = camera.GetProjection();
    const auto eyePos = camera.GetEyePos();
    context.IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

    auto [mesh, vs, ps, hs, ds] = renderable;
    ps.UpdateCb(
        context, dx::cb::GlobalLightingInfo{eyePos, 1, {dx::cb::Light{light_}}},
        dx::cb::PerObjectLightingInfo{dx::cb::Material{smoothness, false}});

    const CbPerObject perObject{world, world * view * projection};

    const CbPerFrame perFrame{eyePos};

    hs.UpdateCb(context, perObject, perFrame);
    ds.UpdateCb(context, perObject, perFrame);

    dx::SetupGpuMesh(context, renderable.Mesh.Get());
    dx::SetupShader(context, vs);
    dx::SetupShader(context, ps);
    dx::SetupShader(context, hs);
    dx::SetupShader(context, ds);
    // auto& predefined = game.Predefined();
    // context.RSSetState(predefined.GetWireFrameOnly());
    context.Draw(64, 0);
}