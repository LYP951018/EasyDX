#pragma once

#include <EasyDx/GameWindow.hpp>
#include <EasyDx/RenderedObject.hpp>
#include <EasyDx/Shaders.hpp>
#include <DirectXMath.h>

class MainWindow : public dx::GameWindow
{
public:
    MainWindow();

protected:
    void Render(ID3D11DeviceContext& context, ID2D1DeviceContext&) override;

private:
    void InitializeObjects();
    void SetUpMatrices();

    DirectX::XMFLOAT4X4 world_, view_, projection_;

    wrl::ComPtr<ID3D11Buffer> constantBuffer_;
    dx::RenderedObject cube_;
    dx::VertexShader vs_;
    wrl::ComPtr<ID3D11PixelShader> ps_;
};