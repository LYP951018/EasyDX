#pragma once

#include <string>
#include <wrl/client.h>
//#include <type_traits>

namespace wrl = Microsoft::WRL;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct IDXGISwapChain;
struct ID3D11Buffer;
struct ID3D10Blob;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;

namespace dx
{
    std::string ws2s(const std::wstring& wstr);

    [[noreturn]]
    void ThrowHRException(long hr);

    void TryHR(long hr);
}
