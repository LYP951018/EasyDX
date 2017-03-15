#pragma once

#include <EasyDx/Scene.hpp>
#include <EasyDx/Common.hpp>
#include <d2d1_1.h>
#include <dwrite_1.h>

class MainScene : public dx::Scene
{
protected:
    void Start() override;
    void Render(ID3D11DeviceContext&, ID2D1DeviceContext& context2D) override;

private:
    wrl::ComPtr<ID2D1SolidColorBrush> brush_;
    wrl::ComPtr<IDWriteTextFormat> textFormat_;
    wrl::ComPtr<IDWriteTextLayout1> textLayout_;
};