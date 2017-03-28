#include "MainScene.hpp"
#include <EasyDX/Game.hpp>
#include <EasyDX/Buffers.hpp>
#include <EasyDx/GameWindow.hpp>
#include <EasyDx/Mesh.hpp>
#include <EasyDx/Texture.hpp>
#include <EasyDx/SimpleVertex.hpp>
#include <gsl/span>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

struct alignas(16) CBChangedPerFrame
{
    DirectX::XMMATRIX World;
    DirectX::XMFLOAT3 LightDir;
};

struct alignas(16) CBImmutablePerFrame
{
    DirectX::XMFLOAT4 LightColor;
};

struct alignas(16) CBMayChanged
{
    DirectX::XMMATRIX View, Projection;
};

void MainScene::Render(ID3D11DeviceContext& context, ID2D1DeviceContext&)
{
    using namespace DirectX;

    auto& game = dx::GetGame();
    auto mainWindow = game.GetMainWindow();
    mainWindow->Clear(Colors::White);

    CBChangedPerFrame cbc = {
        XMMatrixTranspose(cube_.ComputeWorld()),
        { -0.577f, 0.577f, -0.577f }
    };
    context.UpdateSubresource(cbChangedPerFrame.Get(), 0, nullptr, &cbc, 0, 0);
    ID3D11Buffer* const cbs[] = { cbChangedPerFrame.Get(), cbImmutablePerFrame.Get(), cbMayChanged.Get()};
    dx::SetupVSConstantBuffer(context, cbs);
    dx::SetupPSConstantBuffer(context, cbs);
    cube_.Render(context);

    //cbc.World = XMMatrixTranspose(character_.ComputeWorld());
    //context.UpdateSubresource(cbChangedPerFrame.Get(), 0, nullptr, &cbc, 0, 0);
    //character_.Render(context);
}

void MainScene::Start(ID3D11Device& device)
{
    SetupCamera();
    CompileShaders(device);
    InitializeObjects(device);
    InitializeCBs(device);
    RegisterEvents();
}

void MainScene::Destroy() noexcept
{
    auto mainWindow = dx::GetGame().GetMainWindow();
    mainWindow->WindowResize.Remove(resizeHandle_);
    mainWindow->KeyDown.Remove(keyHandle_);
}

void MainScene::InitializeObjects(ID3D11Device& device)
{
    using namespace DirectX;

    dx::SimpleVertex vertices[] = {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), { 0.f, 0.f }},
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), { 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), { 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), { 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), { 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), { 0.f, 1.f } },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) , { 1.f, 0.f } },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), { 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), { 0.f, 0.f } },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), { 0.f, 1.f } },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), { 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), { 1.f, 1.f } },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), { 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), { 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), { 1.f, 0.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), { 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), { 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), { 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), { 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), { 1.f, 1.f } },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), { 0.f, 0.f } },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), { 0.f, 1.f } },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), { 1.f, 0.f } },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), { 1.f, 1.f } },
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

    material_ = std::make_shared<dx::Material>();
    const auto texture = dx::Texture::Load2DFromWicFile(device, LR"(..\Cube\Cat.png)");

    /*character_ = dx::RenderableObject::LoadFromModel(device, LR"(..\Cube\3DModel\figure.FBX)");
    for (auto& mesh : character_.GetMeshes())
    {
        mesh.AttachShaders(vs_, ps_);
    }
    character_.Scale = { 0.5f, 0.5f, 0.5f };*/

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    wrl::ComPtr<ID3D11SamplerState> sampler;
    device.CreateSamplerState(&samplerDesc, sampler.ReleaseAndGetAddressOf());

    material_->MainTexture = { dx::Texture::GetView(device, *texture.Get()), sampler };

    dx::Mesh mesh{
        device,
        gsl::make_span(vertices),
        gsl::make_span(indices),
        material_
    };

    mesh.AttachShaders(vs_, ps_);
    cube_ = { gsl::make_span(&mesh, 1) };

    DirectX::XMStoreFloat4(&cube_.Rotation, DirectX::XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, XM_PIDIV4, 0.f));
}

void MainScene::InitializeCBs(ID3D11Device& device)
{
    cbChangedPerFrame = dx::MakeConstantBuffer<CBChangedPerFrame>(device);
    CBImmutablePerFrame icb = { { 0.5f, 0.5f, 0.5f, 1.0f } };
    cbImmutablePerFrame = dx::MakeConstantBuffer(device, &icb, dx::ResourceUsage::Immutable);
    cbMayChanged = dx::MakeConstantBuffer<CBMayChanged>(device);
}

void MainScene::SetupCamera()
{
    using namespace DirectX;
    auto camera = std::make_unique<dx::Camera>();
    const XMFLOAT3 eye = { 0.f, 14.f, -10.f };
    const XMFLOAT3 at = { 0.f, 1.f, 0.f };
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };
    camera->SetLookAt(eye, at, up);
    SetMainCamera(std::move(camera));
}

void MainScene::CompileShaders(ID3D11Device& device)
{
    vs_ = dx::VertexShader::CompileFromFile(
        device,
        L"Cube.hlsl",
        "VS",
        dx::SimpleVertex::GetLayout()
    );
    ps_ = dx::PixelShader::CompileFromFile(
        device,
        L"Cube.hlsl",
        "PS"
    );
}

void MainScene::RegisterEvents()
{
    auto& game = dx::GetGame();
    auto mainWindow = game.GetMainWindow();

    resizeHandle_ = mainWindow->WindowResize.Add([this](dx::ResizeEventArgs& e) noexcept
    {
        auto& camera = GetMainCamera();
        camera.SetProjection(DirectX::XM_PIDIV4, static_cast<float>(e.NewSize.Width) / e.NewSize.Height, 0.01f, 100.f);
        camera.MainViewport = {
            0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
            0.f, 1.f
        };
        auto& game = dx::GetGame();
        auto& context = game.GetContext3D();
        CBMayChanged cbm = {
            DirectX::XMMatrixTranspose(camera.GetView()),
            DirectX::XMMatrixTranspose(camera.GetProjection())
        };
        context.UpdateSubresource(cbMayChanged.Get(), 0, nullptr, &cbm, 0, 0);
    });

    keyHandle_ = mainWindow->KeyDown.Add([this](dx::KeyEventArgs& e) {
        this->ProcessKey(e);
    });
}

void MainScene::ProcessKey(dx::KeyEventArgs& e)
{
    auto& camera = GetMainCamera();
    auto& game = dx::GetGame();
    auto& context = game.GetContext3D();
    switch (e.Key)
    {
    case 0x57:
        camera.Translate(0.f, 0.f, 0.5f);
        break;
    case 0x41:
        camera.Translate(-0.1f, 0.f, 0.f);
        break;
    default:
        break;
    }
    if (camera.HasChanged())
    {
        CBMayChanged cbm = {
            DirectX::XMMatrixTranspose(camera.GetView()),
            DirectX::XMMatrixTranspose(camera.GetProjection())
        };
        context.UpdateSubresource(cbMayChanged.Get(), 0, nullptr, &cbm, 0, 0);
    }
}
