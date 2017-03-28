#pragma once

#include <EasyDx/Events.hpp>
#include <EasyDx/Scene.hpp>
#include <EasyDx/RenderableObject.hpp>
#include <EasyDx/Shaders.hpp>
#include <EasyDx/Material.hpp>
#include <EasyDx/Camera.hpp>
#include <DirectXMath.h>
//wrl::ComPtr 对于 incomplete type 的支持问题。
#include <d3d11.h>

class MainScene : public dx::Scene
{
protected:
    void Render(ID3D11DeviceContext& context, ID2D1DeviceContext&) override;
    void Start(ID3D11Device&) override;
    void Destroy() noexcept override;

private:
    void InitializeObjects(ID3D11Device&);
    void InitializeCBs(ID3D11Device&);
    void SetupCamera();
    void CompileShaders(ID3D11Device& device);
    void RegisterEvents();
    void ProcessKey(dx::KeyEventArgs& e);

    wrl::ComPtr<ID3D11Buffer> cbChangedPerFrame, cbImmutablePerFrame, cbMayChanged;
    dx::RenderableObject cube_;
    dx::RenderableObject character_;
    dx::VertexShader vs_;
    wrl::ComPtr<ID3D11PixelShader> ps_;
    dx::EventHandle resizeHandle_, keyHandle_;
    std::shared_ptr<dx::Material> material_;
};