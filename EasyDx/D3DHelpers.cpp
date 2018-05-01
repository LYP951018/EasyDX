#include "pch.hpp"
#include "D3DHelpers.hpp"
#include <d3d11.h>

namespace dx
{
    void SetName(ID3D11DeviceChild& child, std::string_view name)
    {
        child.SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.data());
    }
}