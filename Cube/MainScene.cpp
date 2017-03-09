#include "MainScene.hpp"
#include <EasyDX/Game.hpp>
#include <EasyDX/Buffers.hpp>
#include <EasyDx/GameWindow.hpp>
#include <gsl/span>
#include <cstdint>
#include <d3d11.h>
#include <EasyDx/Mesh.hpp>
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
    auto mainWindow = dx::GetGame().GetMainWindow();
    mainWindow->ClearWithDefault();
    SetUpMatrices();

    ConstantBuffer cb;
    using namespace DirectX;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(XMMatrixRotationY(XM_PI / 4.f)));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(XMLoadFloat4x4(&view_)));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(XMLoadFloat4x4(&projection_)));
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
    SetUpMatrices();
    InitializeObjects();

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
}

void MainScene::SetUpMatrices()
{
    using namespace DirectX;
    auto mainWindow = dx::GetGame().GetMainWindow();
    XMStoreFloat4x4(&world_, XMMatrixIdentity());
    const XMFLOAT3 eye = { 0.f, 4.f, -10.f };
    const XMFLOAT3 at = { 0.f, 1.f, 0.f };
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };
    XMStoreFloat4x4(&view_, XMMatrixLookAtLH(
        XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up)
    ));
    XMStoreFloat4x4(&projection_, XMMatrixPerspectiveFovLH(
        XM_PIDIV4, static_cast<float>(mainWindow->GetWidth()) / mainWindow->GetHeight(), 0.01f, 100.f
    ));
}