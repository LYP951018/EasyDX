#include "MainScene.hpp"
#include <EasyDx/Camera.hpp>
#include <EasyDx/Game.hpp>
#include <EasyDx/GameWindow.hpp>
#include <EasyDx/Model.hpp>
#include <EasyDx/Mesh.hpp>
#include <EasyDx/Material.hpp>
#include <EasyDx/Shaders.hpp>
#include <EasyDx/CBStructs.hpp>
#include <EasyDx/Misc.hpp>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3d11.h>
#include "VertexShader.hpp"
#include "PixelShader.hpp"

struct alignas(16) CBPerObject
{
    DirectX::XMMATRIX WorldViewProjection;
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX InvTransposeWorld;
    dx::cb::Material MainMaterial;
};

struct alignas(16) CBPerFrame
{
    DirectX::XMFLOAT4 EyePos;
    dx::cb::Light Dlight;
};

void MainScene::Start(ID3D11Device& device3D)
{
    auto& camera = GetMainCamera();
    using namespace DirectX;
    const auto eye = XMFLOAT3{ 0.0f, 4.0f, -10.0f };
    const auto at = XMFLOAT3{ 0.f, 1.f, 0.f };
    const auto up = XMFLOAT3{ 0.f, 1.f, 0.f };
    camera.SetLookAt(eye, at, up);
    auto mainWindow = dx::GetGame().GetMainWindow();
    resized_ = mainWindow->WindowResize.Add([&](dx::ResizeEventArgs& e) {
        camera.SetProjection(XM_PIDIV4, e.NewSize.GetAspectRatio(), 0.01f, 1000.f);
        camera.MainViewport = {
            0.f, 0.f, static_cast<float>(e.NewSize.Width), static_cast<float>(e.NewSize.Height),
            0.f, 1.f
        };
    });
   
    auto material = dx::Material{
        dx::Smoothness {
            { 0.07568f, 0.61424f, 0.07568f, 1.0f },
            { 0.07568f, 0.61424f, 0.07568f, 1.0f },
            { 0.07568f, 0.61424f, 0.07568f, 1.0f },
            {},
            76.8f
        },
        false
    };
    light_ = dx::PointLight {
        { 1.f, 2.f, 1.f },
        { 1.f, 1.f, 1.f, 1.f },
        { 1.f, 0.08f, 0.0f},
        1000.f,
        true
    };
    cbPerObject_ = dx::MakeConstantBuffer<CBPerObject>(device3D);
    cbPerFrame_ = dx::MakeConstantBuffer<CBPerFrame>(device3D);
    auto vs = dx::VertexShader::FromByteCode(device3D, dx::AsBytes(TheVertexShader), dx::SimpleVertex::GetLayout());
    auto ps = dx::PixelShader::FromByteCode(device3D, dx::AsBytes(ThePixelShader));
    for (auto cb : { cbPerObject_, cbPerFrame_ })
    {
        vs.ConstantBuffers.push_back(cb);
        ps.ConstantBuffers.push_back(cb);
    }

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

    /*dx::SimpleMeshDataView meshData = { gsl::make_span(vertices), gsl::make_span(indices) };
    ball_.Meshes.push_back(
        dx::Mesh{ device3D,
        meshData,
        std::make_shared<dx::Material>(material), vs, ps }
    );*/
    DirectX::XMStoreFloat4(&ball_.Transform.Rotation, DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f), DirectX::XM_PI / 6.f));
    dx::SimpleMeshData meshData;
    dx::MakeUVSphere(1.f, 20, 20, meshData);
    ball_.Meshes.push_back(dx::Mesh{ device3D,
        dx::SimpleMeshDataView::FromMeshData(meshData),
        std::make_shared<dx::Material>(material), vs, ps });
}


void MainScene::Render(ID3D11DeviceContext& context3D, ID2D1DeviceContext&)
{
    dx::GetGame().GetMainWindow()->Clear(DirectX::Colors::White);
    const auto& camera = GetMainCamera();
    const auto world = DirectX::XMMatrixRotationY(DirectX::XM_PI / 6.f);//dx::ComputeWorld(ball_.Transform);
    const auto worldViewProj = world * camera.GetView() * camera.GetProjection();
    auto cb = CBPerObject{
        worldViewProj,
        world,
        DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse({}, world)),
    };
    auto cbFrame = CBPerFrame{
        dx::MakePosition(camera.GetEyePos()),
        dx::cb::Light::FromPoint(light_)
    };
    context3D.UpdateSubresource(cbPerFrame_.Get(), {}, {}, &cbFrame, {}, {});
    ball_.Render(context3D, [&, this](const dx::Mesh& mesh) 
    {
        const auto& material = *mesh.GetMaterial();
        cb.MainMaterial = dx::cb::Material::FromPlain(material.Smooth);
        context3D.UpdateSubresource(this->cbPerObject_.Get(), {}, {}, &cb, {}, {});
    });
}

void MainScene::Destroy() noexcept
{

}