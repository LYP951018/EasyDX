#include "MainWindow.hpp"
#include <Game.hpp>
#include <DXHelpers.hpp>
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

MainWindow::MainWindow()
{
    SetUpBuffers();
   
    CompileShaders();
    SetUpLayout();
}

void MainWindow::Render(ID3D11DeviceContext& context, ID2D1DeviceContext&)
{
    ClearWithDefault();
    SetUpMatrices();
    context.IASetInputLayout(vertexLayout_.Get());

    ID3D11Buffer* const vertexBuffers[] = { vertexBuffer_.Get() };
    const UINT vertexBufferStrides[] = { sizeof(Vertex) };
    const UINT vertexBufferOffsets[] = { 0 };
    context.IASetVertexBuffers(0, std::size(vertexBuffers), vertexBuffers, vertexBufferStrides, vertexBufferOffsets);

    context.IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
    context.VSSetShader(vertexShader_.Get(), nullptr, 0);
    context.VSSetConstantBuffers(0, 1, cbs);
    context.PSSetShader(pixelShader_.Get(), nullptr, 0);
    context.PSSetConstantBuffers(0, 1, cbs);

    context.DrawIndexed(36, 0, 0);
}

void MainWindow::SetUpBuffers()
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

    auto& d3dDevice = dx::GetD3DDevice();
    vertexBuffer_ = dx::MakeVertexBuffer(d3dDevice, gsl::make_span(vertices));

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

    indexBuffer_ = dx::MakeIndexBuffer(d3dDevice, gsl::make_span(indices));

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    dx::TryHR(d3dDevice.CreateBuffer(&constantBufferDesc, nullptr, constantBuffer_.GetAddressOf()));
}

void MainWindow::SetUpMatrices()
{
    using namespace DirectX;
    XMStoreFloat4x4(&world_, XMMatrixIdentity());
    const XMFLOAT3 eye = { 0.f, 4.f, -10.f };
    const XMFLOAT3 at = { 0.f, 1.f, 0.f };
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };
    XMStoreFloat4x4(&view_, XMMatrixLookAtLH(
        XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up)
    ));
    XMStoreFloat4x4(&projection_, XMMatrixPerspectiveFovLH(
        XM_PIDIV4, static_cast<float>(GetWidth()) / GetHeight(), 0.01f, 100.f
    ));
}

void MainWindow::SetUpLayout()
{
    const D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto& device = dx::GetD3DDevice();
    dx::TryHR(device.CreateInputLayout(layout, std::size(layout), vertexShaderByteCode_->GetBufferPointer(),
        vertexShaderByteCode_->GetBufferSize(), vertexLayout_.GetAddressOf()));
}

void MainWindow::CompileShaders()
{
    vertexShaderByteCode_ = dx::CompileShaderFromFile(L"Cube.hlsl", "VS", "vs_4_0");
    auto& device = dx::GetD3DDevice();
    vertexShader_ = dx::CreateVertexShader(device, *vertexShaderByteCode_.Get());
    auto psBlob = dx::CompileShaderFromFile(L"Cube.hlsl", "PS", "ps_4_0");
    pixelShader_ = dx::CreatePixelShader(device, *psBlob.Get());
}
