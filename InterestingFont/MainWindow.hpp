#pragma once

#include <EasyDx/GameWindow.hpp>

class MainWindow : public dx::GameWindow
{
public:
    MainWindow();

protected:
    void Render(ID3D11DeviceContext&, ID2D1DeviceContext& context2D) override;

private:
    wrl::ComPtr<ID2D1SolidColorBrush> brush_;
    wrl::ComPtr<IDWriteTextFormat> textFormat_;
    wrl::ComPtr<IDWriteTextLayout1> textLayout_;
};