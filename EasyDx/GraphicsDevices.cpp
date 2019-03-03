#include "pch.hpp"
#include "GraphicsDevices.hpp"
#include <d3d11.h>

namespace dx
{

    DeviceContext3DPair MakeDevice3D()
    {
        DeviceContext3DPair pair;
        UINT creationFlags = {};
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        creationFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        const D3D_FEATURE_LEVEL featureLevel[] = {D3D_FEATURE_LEVEL_11_0};

        TryHR(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
                                featureLevel, gsl::narrow<UINT>(std::size(featureLevel)),
                                D3D11_SDK_VERSION, pair.first.ReleaseAndGetAddressOf(), nullptr,
                                pair.second.ReleaseAndGetAddressOf()));
        return pair;
    }

} // namespace dx
