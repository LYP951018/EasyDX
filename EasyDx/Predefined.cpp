#include "pch.hpp"
#include "Predefined.hpp"
#include "Renderable.hpp"
#include <d3d11.h>
#include <DirectXtk/DirectXHelpers.h>

namespace dx
{
    Predefined::Predefined(ID3D11Device& device)
    {
        MakeWhiteTex(device);
        MakeBasicVS(device);
        MakeBasicPS(device);
    }

    Rc<Renderable> Predefined::GetRenderable(gsl::span<SimpleVertex> vertices, gsl::span<std::uint16_t> indices) const
    {
        auto vs = basicVS_;
        vs.Cbs.push_back(vsCb_.second);
        auto ps = basicPS_;
        ps.Cbs.push_back(psPerObjectLightingCb_.second);
        return MakeComponent<Renderable>(std::move(vs), std::move(ps), SimpleCpuMeshView{ vertices, indices }, std::vector<Rc<IConstantBuffer>>{ vsCb_.first, psPerObjectLightingCb_.first });
    }

    void Predefined::MakeWhiteTex(ID3D11Device& device)
    {
        std::uint32_t pixels[kDefaultTexHeight * kDefaultTexWidth];
        std::fill(pixels, pixels + std::size(pixels), static_cast<std::uint32_t>(-1));
        wrl::ComPtr<ID3D11Texture2D> tex;
        CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R32G32B32A32_FLOAT, kDefaultTexWidth, kDefaultTexHeight };
        D3D11_SUBRESOURCE_DATA resourceData{};
        resourceData.pSysMem = pixels;
        device.CreateTexture2D(&desc, &resourceData, tex.GetAddressOf());
        wrl::ComPtr<ID3D11ShaderResourceView> view;
        CD3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{ D3D_SRV_DIMENSION_TEXTURE2D };
        device.CreateShaderResourceView(tex.Get(), &viewDesc, view.GetAddressOf());
        white_ = MakeComponent<Texture>(device, std::move(tex), std::move(view));
    }

    void Predefined::MakeBasicVS(ID3D11Device& device)
    {
        basicVS_ = MakeVS(device, "BasicVS.hlsl");
        basicVS_.Cbs.push_back(vsCb_.fir)
    }

    void Predefined::MakeBasicPS(ID3D11Device& device)
    {
        basicPS_ = MakePS(device, "BasicPS.hlsl");
    }

    VertexShader Predefined::MakeVS(ID3D11Device& device, const char * fileName)
    {
        return VertexShader::CompileFromFile(device, fs::current_path() / "Shaders" / fileName, "main", SimpleVertex::GetLayout());
    }

    PixelShader Predefined::MakePS(ID3D11Device& device, const char* fileName)
    {
        return PixelShader::CompileFromFile(device, fs::current_path() / "Shaders" / fileName, "main");
    }
}
