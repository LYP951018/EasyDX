#include "MainScene.hpp"
#include <EasyDX/Game.hpp>
#include <EasyDX/Buffers.hpp>
#include <EasyDx/GameWindow.hpp>
#include <EasyDx/Mesh.hpp>
#include <EasyDx/Camera.hpp>
#include <EasyDx/Texture.hpp>
#include <EasyDx/SimpleVertex.hpp>
#include <gsl/span>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

struct ConstantBuffer
{
    DirectX::XMFLOAT4X4 World, View, Projection;
    DirectX::XMFLOAT4 LightDir, LightColor;
};

void MainScene::Render(ID3D11DeviceContext& context, ID2D1DeviceContext&)
{
    using namespace DirectX;

    auto& game = dx::GetGame();
    auto mainWindow = game.GetMainWindow();

    mainWindow->Clear(Colors::White);

    ConstantBuffer cb;
    const auto& camera = GetMainCamera();
    XMStoreFloat4x4(&cb.World, XMMatrixTranspose(cube_.ComputeWorld()));
    XMStoreFloat4x4(&cb.View, XMMatrixTranspose(camera.GetView()));
    XMStoreFloat4x4(&cb.Projection, XMMatrixTranspose(camera.GetProjection()));
    cb.LightDir = { -0.577f, 0.577f, -0.577f, 1.0f };
    cb.LightColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    //XMStoreFloat4(&cb.Color, DirectX::Colors::Black);
    context.UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &cb, 0, 0);

    ID3D11Buffer* const cbs[] = { constantBuffer_.Get() };
    context.VSSetConstantBuffers(0, 1, cbs);
    context.PSSetConstantBuffers(0, 1, cbs);

    cube_.Render(context);
    XMStoreFloat4x4(&cb.World, XMMatrixTranspose(character_.ComputeWorld()));
    context.UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &cb, 0, 0);
    character_.Render(context);
}

void MainScene::Start()
{
    using namespace DirectX;

    auto camera = std::make_unique<dx::Camera>();

    const XMFLOAT3 eye = { 0.f, 14.f, -10.f };
    const XMFLOAT3 at = { 0.f, 1.f, 0.f };
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };

    camera->SetUvn(eye, at, up);

    SetMainCamera(std::move(camera));

    auto& d3dDevice = dx::GetGame().GetDevice3D();
    vs_ = dx::VertexShader::CompileFromFile(
        d3dDevice,
        L"Cube.hlsl",
        "VS",
        dx::SimpleVertex::GetLayout()
    );
    ps_ = dx::PixelShader::CompileFromFile(
        d3dDevice,
        L"Cube.hlsl",
        "PS"
    );

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

    auto& game = dx::GetGame();
    auto& d3dDevice = game.GetDevice3D();

    material_ = std::make_shared<dx::Material>();
    const auto texture = dx::Texture::Load2DFromWicFile(d3dDevice, LR"(..\Cube\Cat.png)");

    character_ = dx::RenderableObject::LoadFromModel(d3dDevice, LR"(..\Cube\3DModel\figure.FBX)");
    for (auto& mesh : character_.GetMeshes())
    {
        mesh.AttachShaders(vs_, ps_);
    }
    character_.Scale = { 0.5f, 0.5f, 0.5f };

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    wrl::ComPtr<ID3D11SamplerState> sampler;
    d3dDevice.CreateSamplerState(&samplerDesc, sampler.ReleaseAndGetAddressOf());

    material_->MainTexture = { dx::Texture::GetView(d3dDevice, *texture.Get()), sampler };

    dx::Mesh mesh{
        d3dDevice,
        gsl::make_span(vertices),
        gsl::make_span(indices),
        material_
    };

    mesh.AttachShaders(vs_, ps_);

    cube_ = { gsl::make_span(&mesh, 1) };
    constantBuffer_ = dx::MakeConstantBuffer<ConstantBuffer>(d3dDevice);

   DirectX::XMStoreFloat4(&cube_.Rotation, DirectX::XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, XM_PIDIV4, 0.f));
}