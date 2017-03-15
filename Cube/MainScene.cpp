#include "MainScene.hpp"
#include <EasyDX/Game.hpp>
#include <EasyDX/Buffers.hpp>
#include <EasyDx/GameWindow.hpp>
#include <EasyDx/Mesh.hpp>
#include <EasyDx/Camera.hpp>
#include <gsl/span>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

struct Vertex
{
    DirectX::XMFLOAT3 Pos, Normal;
};

struct ConstantBuffer
{
    DirectX::XMFLOAT4X4 world, view, projection;
    DirectX::XMFLOAT4 LightDir, LightColor;
    DirectX::XMFLOAT4 Color;
};

void MainScene::Render(ID3D11DeviceContext& context, ID2D1DeviceContext&)
{
    using namespace DirectX;

    auto& game = dx::GetGame();
    auto mainWindow = game.GetMainWindow();

    mainWindow->Clear(Colors::White);

    ConstantBuffer cb;
    const auto& camera = GetMainCamera();
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(cube_.ComputeWorld()));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(camera.GetView()));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(camera.GetProjection()));
    cb.LightDir = { -0.577f, 0.577f, -0.577f, 1.0f };
    cb.LightColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    XMStoreFloat4(&cb.Color, DirectX::Colors::Black);
    context.UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &cb, 0, 0);

    ID3D11Buffer* const cbs[] = { constantBuffer_.Get() };
    context.VSSetConstantBuffers(0, 1, cbs);
    context.PSSetConstantBuffers(0, 1, cbs);

    cube_.AttachVertexShader(vs_);
    cube_.AttachPixelShader(ps_);
    cube_.Render(context);
}

void MainScene::Start()
{
    using namespace DirectX;

    auto camera = std::make_unique<dx::Camera>();

    const XMFLOAT3 eye = { 0.f, 4.f, -10.f };
    const XMFLOAT3 at = { 0.f, 1.f, 0.f };
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };

    camera->SetUvn(eye, at, up);

    SetMainCamera(std::move(camera));

    InitializeObjects();

    auto& game = dx::GetGame();
    auto mainWindow = game.GetMainWindow();

    eventHandles_.push_back(mainWindow->WindowResize.Add([this](dx::ResizeEventArgs& e) noexcept
    {
        auto& camera = GetMainCamera();
        camera.SetProjection(XM_PIDIV4, static_cast<float>(e.NewSize.Width) / e.NewSize.Height, 0.01f, 100.f);
        camera.MainViewport = {
            0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
            0.f, 1.f
        };
    }));

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto& d3dDevice = dx::GetGame().GetDevice3D();
    vs_ = dx::VertexShader::CompileFromFile(
        d3dDevice,
        L"Cube.hlsl",
        "VS",
        gsl::make_span(layout)
    );
    ps_ = dx::PixelShader::CompileFromFile(
        d3dDevice,
        L"Cube.hlsl",
        "PS"
    );
}

void MainScene::Destroy() noexcept
{
    auto mainWindow = dx::GetGame().GetMainWindow();
    for (auto eventHandle : eventHandles_)
    {
        //FIXME: 要用户记录 Event 类型与索引之间的对应关系非常麻烦。
        //好像没有什么太好的方案？
        mainWindow->WindowResize.Remove(eventHandle);
    }
}

void MainScene::InitializeObjects()
{
    using namespace DirectX;

    Vertex vertices[] = {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    std::uint16_t indices[] = {
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
    };

    auto& game = dx::GetGame();
    auto& d3dDevice = game.GetDevice3D();

    dx::Mesh mesh{
        d3dDevice,
        gsl::make_span(vertices),
        gsl::make_span(indices)
    };

    cube_ = { gsl::make_span(&mesh, 1) };
    constantBuffer_ = dx::MakeConstantBuffer<ConstantBuffer>(d3dDevice);

    DirectX::XMStoreFloat4(&cube_.Rotation, DirectX::XMQuaternionRotationRollPitchYaw(0.f, XM_PIDIV4, 0.f));
}