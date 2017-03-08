#include "MainWindow.hpp"
#include <string_view>
#include <EasyDx/Game.hpp>
#include <d2d1_1.h>
#include <dwrite_1.h>

MainWindow::MainWindow()
{
    auto& game = dx::GetGame();
    auto& context2D = game.GetContext2D();
    dx::TryHR(context2D.CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 1.f), brush_.ReleaseAndGetAddressOf()));
    
    auto& dwFactory = game.GetDWriteFactory();
    dx::TryHR(dwFactory.CreateTextFormat(L"Source Code Pro",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.f,
        L"zh-CN",
        textFormat_.ReleaseAndGetAddressOf()));

    const std::wstring_view str = L"Aji chvim 😭";

    wrl::ComPtr<IDWriteTextLayout> layout;

    dx::TryHR(dwFactory.CreateTextLayout(
        str.data(), static_cast<UINT>(str.size()), textFormat_.Get(), 300.f, 100.f, layout.ReleaseAndGetAddressOf()));

    layout.As(&textLayout_);
}

void MainWindow::Render(ID3D11DeviceContext&, ID2D1DeviceContext& context2D)
{
    context2D.BeginDraw();
    context2D.DrawRectangle(D2D1::RectF(20.f, 20.f, 200.f, 200.f),
        brush_.Get());
    context2D.DrawTextLayout(
        D2D1::Point2F(100.f, 100.f),
        textLayout_.Get(),
        brush_.Get()
    );
    dx::TryHR(context2D.EndDraw());
}