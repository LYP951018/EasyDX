#pragma once

#include <GameWindow.hpp>
#include <DirectXMath.h>

class MainWindow : public dx::GameWindow
{
public:
    MainWindow();

protected:
    void Render(ID3D11DeviceContext& context) override;

private:
    void SetUpBuffers();
    void SetUpMatrices();
    void SetUpLayout();
    void CompileShaders();

    DirectX::XMFLOAT4X4 world_, view_, projection_;

    wrl::ComPtr<ID3D11InputLayout> vertexLayout_;
    wrl::ComPtr<ID3D11Buffer> vertexBuffer_;
    wrl::ComPtr<ID3D11Buffer> indexBuffer_;
    wrl::ComPtr<ID3D11Buffer> constantBuffer_;
    wrl::ComPtr<ID3D10Blob> vertexShaderByteCode_;
    wrl::ComPtr<ID3D11VertexShader> vertexShader_;
    wrl::ComPtr<ID3D11PixelShader> pixelShader_;
};