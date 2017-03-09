#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/RenderedObject.hpp>
#include <EasyDx/Shaders.hpp>
#include <DirectXMath.h>
//wrl::ComPtr 对于 incomplete type 的支持问题。
#include <d3d11.h>

class MainScene : public dx::Scene
{
public:
    MainScene() = default;

protected:
    void Render(ID3D11DeviceContext& context, ID2D1DeviceContext&) override;
    void Start() override;

private:
    void InitializeObjects();
    void SetUpMatrices();

    DirectX::XMFLOAT4X4 world_, view_, projection_;

    wrl::ComPtr<ID3D11Buffer> constantBuffer_;
    dx::RenderedObject cube_;
    dx::VertexShader vs_;
    wrl::ComPtr<ID3D11PixelShader> ps_;
};