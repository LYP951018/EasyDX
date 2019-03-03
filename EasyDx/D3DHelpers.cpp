#include "pch.hpp"
#include "D3DHelpers.hpp"
#include <d3d11.h>

namespace dx
{
    void SetName(ID3D11DeviceChild& child, std::string_view name)
    {
        child.SetPrivateData(WKPDID_D3DDebugObjectName, gsl::narrow<std::uint32_t>(name.size()),
                             name.data());
    }

    wrl::ComPtr<ID3D11RenderTargetView> MakeRtFromTex2D(ID3D11Device& device3D,
                                                        ID3D11Texture2D& texture2D)
    {
        wrl::ComPtr<ID3D11RenderTargetView> rt;
        D3D11_RENDER_TARGET_VIEW_DESC desc{};
        // If the format is set to DXGI_FORMAT_UNKNOWN, then the format of the resource that the
        // view binds to the pipeline will be used.
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        device3D.CreateRenderTargetView(&texture2D, &desc, rt.GetAddressOf());
        return rt;
    }
} // namespace dx